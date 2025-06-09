import sounddevice as sd
import vosk, queue, json
import socket
import locale
#print(locale.getpreferredencoding())

from struct import *

HOST = "127.0.0.1" 
PORT = 65000  
BUFF_SIZE = 64
ctr = 0
data = bytearray()

q = queue.Queue()

devices = sd.query_devices()
#print("Select device id: \n", devices)

dev_id = 1 # default


samplerate = int(sd.query_devices(dev_id, 'input')['default_samplerate'])

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

path = "models\\vosk-model-small-ru-0.22"
Delta = 0
a = [0,0,0]
g = [0,0,0]
isEnd = False
action = 0
direction = 0
digit = 0 
try:
    model = vosk.Model(path)
    with sd.RawInputStream(samplerate=samplerate, blocksize=8000, device=dev_id, dtype='int16', channels=1, callback=(lambda i, f, t, s: q.put(bytes(i)))):
        rec = vosk.KaldiRecognizer(model, samplerate)
        while not isEnd:
            data = s.recv(BUFF_SIZE)
            ##print(f"{data.hex()}")
            Delta, g[0], g[1], g[2], a[0], a[1], a[2], isEnd = unpack('>qhhhhhh?', data)
       
            Delta = Delta / 1000000
            g[0] = g[0] / 16384
            g[1] = g[1] / 16384
            g[2] = g[2] / 16384
            a[0] = a[0] / 16384
            a[1] = a[1] / 16384
            a[2] = a[2] / 16384
            #print(f"Received {Delta:>6.4f}, {g[0]:>7.3f}, {g[1]:>7.03}, {g[2]:>7.3f}, {a[0]:>7.3f}, {a[1]:>7.3f}, {a[2]:>7.3f}")
            
            vdata = q.get()
            if rec.AcceptWaveform(vdata):
                vdata = json.loads(rec.Result())["text"]
                print("Recognized: " + vdata)
                #forward, backward, left, right, faster, slower, digits
                if("все" in vdata):
                    isEnd = True
                if("стой" in vdata or "стоять" in vdata or "стоит" in vdata or "той" in vdata or "ой" in vdata):
                    action = 0 
                elif("быстрей" in vdata or "быстрее" in vdata):
                    action = 1
                elif("притормози" in vdata or "медленнее" in vdata or "немедля" in vdata or "тише" in vdata or "внушить" in vdata or "ниши" in vdata):
                    action = 2
                elif("скорость" in vdata):
                    action = 3
                elif("вперёд" in vdata or "перед" in vdata):
                    action = 4
                elif("назад" in vdata):
                    action = 5
                else:
                    action = 6
                
                if("прямо" in vdata):
                    direction = 0
                elif("влево" in vdata or "лево" in vdata or "врио" in vdata):
                    direction = 1
                elif("вправо" in vdata or "право" in vdata or "права" in vdata or "справа" in vdata or "праву" in vdata or "правом" in vdata):
                    direction = 2
                else:
                    direction = 3

                if("один" in vdata or "единица" in vdata or "единице" in vdata or "единицы" in vdata):
                    digit = 1
                elif("два" in vdata or "двойка" in vdata):
                    digit = 2
                elif("три" in vdata or "при" in vdata or "ри" in vdata or "тройка" in vdata or "бойко" in vdata or "которой" in vdata):
                    digit = 3
                elif("четыре" in vdata or "четвёрка" in vdata or "йорка" in vdata):
                    digit = 4
                elif("пять" in vdata or "опять" in vdata or "пятёрка" in vdata):
                    digit = 5
                elif("шесть" in vdata or "шест" in vdata or "шестёрка" in vdata or "шестёркой" in vdata):
                    digit = 6
                elif("семь" in vdata or "семёрка" in vdata):
                    digit = 7
                elif("восемь" in vdata or "восьмёрка" in vdata or "восьмёрку" in vdata):
                    digit = 8
                elif("девять" in vdata or "девятка" in vdata):
                    digit = 9
                elif("десять" in vdata or "десятка" in vdata):
                    digit = 10
                elif("ноль" in vdata):
                    digit = 0   
                else:
                    digit = 0
            #else:
                #vdata = json.loads(rec.PartialResult())["partial"]
                #if vdata != "":
                    #print(vdata)

            to_send = pack('>IBBB?', ctr, action, direction, digit, isEnd)
            s.send(to_send)
            ctr += 1
            ctr %= 256**4
            action = 6
            direction = 3
            digit = 0
except KeyboardInterrupt:
    print('\nDone')
        
s.close()
