import sounddevice as sd
import vosk, queue, json
import socket
import locale
import os
import torch
import torch.nn as nn
import numpy as np
import librosa
from torchvision import transforms
import pyaudio
import time
from collections import deque
import threading
import io

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from PIL import Image

print(locale.getpreferredencoding())

from struct import *

# Определение архитектуры модели
class CRNN(nn.Module):
    def __init__(self, num_classes):
        super(CRNN, self).__init__()
        self.cnn = nn.Sequential(
            nn.Conv2d(1, 32, kernel_size=3, stride=1, padding=1),
            nn.BatchNorm2d(32),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=2, stride=2),

            nn.Conv2d(32, 64, kernel_size=3, stride=1, padding=1),
            nn.BatchNorm2d(64),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=2, stride=2),

            nn.Conv2d(64, 128, kernel_size=3, stride=1, padding=1),
            nn.BatchNorm2d(128),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=(2, 2), stride=(2, 2)),
        )
        self.rnn = nn.GRU(
            input_size=128 * 8,
            hidden_size=128,
            num_layers=2,
            batch_first=True,
            bidirectional=True
        )
        self.fc = nn.Linear(256, num_classes)

    def forward(self, x):
        x = self.cnn(x)
        batch, channels, height, width = x.size()
        x = x.permute(0, 3, 1, 2)
        x = x.reshape(batch, width, channels * height)
        x, _ = self.rnn(x)
        x = x[:, -1, :]
        x = self.fc(x)
        return x


def load_model(model_path, num_classes=10):
    model = CRNN(num_classes=num_classes)
    checkpoint = torch.load(model_path, map_location=torch.device('cpu'))
    model.load_state_dict(checkpoint['model_state_dict'])
    model.eval()
    return model


def trim_silence(samples, sample_rate, top_db=25, min_duration=0.2):
    """Обрезка тишины с проверкой минимальной длины"""
    if len(samples) == 0:
        return samples

    trimmed, _ = librosa.effects.trim(samples, top_db=top_db)

    if len(trimmed) / sample_rate < min_duration:
        start = max(0, len(samples) // 2 - int(min_duration * sample_rate // 2))
        end = start + int(min_duration * sample_rate)
        return samples[start:end]

    return trimmed


def preprocess_segment(segment, sample_rate, params):
    """Создание спектрограммы для аудио сегмента"""
    try:
        if len(segment) == 0:
            return None

        mel_spec = librosa.feature.melspectrogram(
            y=segment,
            sr=sample_rate,
            n_fft=params['n_fft'],
            hop_length=params['hop_length'],
            n_mels=params['n_mels'],
            fmin=0,
            fmax=8000
        )
        log_mel_spec = librosa.power_to_db(mel_spec, ref=np.max)

        target_length = params['max_time_steps']
        if log_mel_spec.shape[1] < target_length:
            pad_width = target_length - log_mel_spec.shape[1]
            log_mel_spec = np.pad(log_mel_spec, ((0, 0), (0, pad_width)), mode='constant')
        elif log_mel_spec.shape[1] > target_length:
            start = (log_mel_spec.shape[1] - target_length) // 2
            log_mel_spec = log_mel_spec[:, start:start + target_length]

        return log_mel_spec
    except:
        return None


class AudioProcessor:
    """Класс для обработки аудиопотока с микрофона"""

    def __init__(self, model, params):
        self.model = model
        self.params = params
        self.sample_rate = params['sample_rate']
        self.chunk_size = int(self.sample_rate * 1.0)  # 1 секунда аудио
        self.buffer = deque(maxlen=int(self.sample_rate * 1))  # 3-секундный буфер
        self.recording = False
        self.audio = pyaudio.PyAudio()
        self.silence_threshold = 0.005  # Порог громкости
        self.silence_window = int(0.5 * self.sample_rate)  # Окно для вычисления RMS
        self.current_prediction = -1  # Текущее распознанное число (-1 - ничего не распознано)
        self.prediction_lock = threading.Lock()

    def start(self):
        """Запуск обработки аудиопотока"""
        self.recording = True
        self.buffer.clear()

        # Поток для записи аудио
        self.stream = self.audio.open(
            format=pyaudio.paInt16,
            channels=1,
            rate=self.sample_rate,
            input=True,
            frames_per_buffer=self.chunk_size,
            stream_callback=self.audio_callback
        )

        # Поток для обработки данных
        self.processing_thread = threading.Thread(target=self.process_buffer)
        self.processing_thread.daemon = True
        self.processing_thread.start()

        #print("Слушаю микрофон... Нажмите Ctrl+C для остановки")

    def stop(self):
        """Остановка обработки аудиопотока"""
        self.recording = False
        self.stream.stop_stream()
        self.stream.close()
        self.processing_thread.join()

    def audio_callback(self, in_data, frame_count, time_info, status):
        """Callback-функция для получения аудиоданных"""
        self.buffer.extend(np.frombuffer(in_data, dtype=np.int16))
        return (in_data, pyaudio.paContinue)

    def is_silence(self, audio_data):
        """Проверка, является ли аудио сегмент тишиной"""
        if len(audio_data) < self.silence_window:
            return True

        # Разделение на фреймы для анализа
        frames = librosa.util.frame(
            audio_data,
            frame_length=self.silence_window,
            hop_length=self.silence_window
        )

        # Вычисление RMS для каждого фрейма
        rms_values = np.sqrt(np.mean(frames ** 2, axis=0))

        # Проверка максимальной громкости
        return np.max(rms_values) < self.silence_threshold

    def predict_audio_segment(self, segment, params):
        """Предсказание цифры для аудио сегмента"""
        if len(segment) == 0:
            return None, 0.0

        # Создание спектрограммы
        spec = preprocess_segment(segment, params['sample_rate'], params)
        if spec is None:
            return None, 0.0

        # Преобразование в изображение и тензор
        plt.figure(figsize=(4, 4), dpi=100)
        plt.axis('off')
        plt.imshow(spec, aspect='auto', origin='lower', cmap='viridis')
        buffer = io.BytesIO()
        plt.savefig(buffer, format='png', bbox_inches='tight', pad_inches=0)
        plt.close()
        buffer.seek(0)
        img = Image.open(buffer).convert('RGB')

        # Трансформации
        transform = transforms.Compose([
            transforms.Resize((params['n_mels'], params['max_time_steps'])),
            transforms.Grayscale(num_output_channels=1),
            transforms.ToTensor(),
            transforms.Normalize(mean=[0.5], std=[0.5])
        ])

        tensor = transform(img).unsqueeze(0)

        # Предсказание
        with torch.no_grad():
            output = self.model(tensor)
        probabilities = torch.softmax(output, dim=1)
        confidence, predicted = torch.max(probabilities, 1)

        return predicted.item(), confidence.item()

    def process_buffer(self):
        """Обработка аудиобуфера и распознавание цифр"""
        min_segment_length = int(self.sample_rate * 0.5)  # 0.5 секунд

        while self.recording:
            if len(self.buffer) < self.chunk_size:
                time.sleep(0.1)
                continue

            # Получение данных из буфера
            audio_data = np.array(self.buffer)
            # Очищаем только обработанную часть
            for _ in range(len(self.buffer) - self.buffer.maxlen + self.chunk_size):
                if self.buffer:
                    self.buffer.popleft()

            # Конвертация в float и нормализация
            audio_data = audio_data.astype(np.float32) / 32768.0

            # 1. Проверка громкости перед обработкой
            if self.is_silence(audio_data):
                with self.prediction_lock:
                    self.current_prediction = -1
                continue

            # Обрезка тишины
            audio_data = trim_silence(audio_data, self.sample_rate)

            # 2. Дополнительная проверка после обрезки
            if len(audio_data) < min_segment_length or self.is_silence(audio_data):
                with self.prediction_lock:
                    self.current_prediction = -1
                continue

            # Распознавание
            temp, confidence = self.predict_audio_segment(audio_data, self.params)

            # Фильтрация результатов
            if confidence > 0.90:  # Порог уверенности
                with self.prediction_lock:
                    self.current_prediction = temp
                print(f"Распознано: {temp} (уверенность: {confidence:.1%})")
            else:
                with self.prediction_lock:
                    self.current_prediction = -1

    def get_current_prediction(self):
        """Получение текущего распознанного числа (для интеграции с C++)"""
        with self.prediction_lock:
            return self.current_prediction

# Основной процесс
if __name__ == "__main__":
    HOST = "127.0.0.1" 
    PORT = 65000  
    BUFF_SIZE = 64
    ctr = 0

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    
    Delta = 0
    a = [0,0,0]
    g = [0,0,0]
    isEnd = False
    # Загрузка параметров
    params_path = 'D:\\models\\digit\\audio_params.npz'
    if os.path.exists(params_path):
        params_data = np.load(params_path)
        params = {key: params_data[key].item() for key in params_data.files}
    else:
        params = {
            'sample_rate': 16000,
            'n_fft': 1024,
            'hop_length': 512,
            'n_mels': 64,
            'max_time_steps': 128
        }

    # Загрузка модели
    model_path = 'models\\digit\\best_model.pth'
    if not os.path.exists(model_path):
        raise FileNotFoundError(f"Модель {model_path} не найдена")

    model = load_model(model_path, num_classes=10)

    # Инициализация и запуск обработки аудио
    processor = AudioProcessor(model, params)
    try:
        processor.start()
        while processor.recording and not isEnd:
            command = 0
            processor.digit = 0
            action = 6
            direction = 3
            digit = 0
            data = s.recv(BUFF_SIZE)
    
            Delta, g[0], g[1], g[2], a[0], a[1], a[2], isEnd = unpack('>qhhhhhh?', data)
       
            Delta = Delta / 1000000
            g[0] = g[0] / 16384
            g[1] = g[1] / 16384
            g[2] = g[2] / 16384
            a[0] = a[0] / 16384
            a[1] = a[1] / 16384
            a[2] = a[2] / 16384
            #print(f"Received {Delta:>6.4f}, {g[0]:>7.3f}, {g[1]:>7.03}, {g[2]:>7.3f}, {a[0]:>7.3f}, {a[1]:>7.3f}, {a[2]:>7.3f}")

            command = int(processor.current_prediction)
            if(command == -1):
                command = 0
            if(command != 0):
                print(command)
            if(command == 0):
                action = 6
                direction = 3
                digit = 0
            elif(command == 1): #Stop
                action = 0
                direction = 0
                digit = 0
            elif(command == 2): #Speed
                action = 3
                direction = 3
                digit = 6
            elif(command == 3): #Move forward
                action = 4
                direction = 0
                digit = 0
            elif(command == 4): #Move backward
                action = 5
                direction = 0
                digit = 0
            elif(command == 5): #Move left
                action = 6
                direction = 1
                digit = 0
            elif(command == 6): #Move right
                action = 6
                direction = 2
                digit = 0
            elif(command == 7):
                action = 6
                direction = 3
                digit = 0
            elif(command == 8):
                action = 6
                direction = 3
                digit = 0
            elif(command == 9): # End
                action = 6
                direction = 3
                digit = 0
                isEnd = True

            to_send = pack('>IBBB?', ctr, action, direction, digit, isEnd)
            s.send(to_send)
            ctr += 1
            ctr %= 256**4
    except KeyboardInterrupt:
        print("\nОстановка...")
    finally:
        processor.stop()

s.close()
