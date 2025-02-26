#include <windows.h>
#include <iostream>
#include <conio.h>

#include "signal_source.h"

using namespace std;
void Draw(HDC hdc, RECT rc, COLORREF color, double f1, double f2, double p, double a1, double a2);
int main()
{
    HWND console = GetConsoleWindow();
    HDC hdc = GetDC(console);

    RECT rc;
    string input;
    bool exit = false;
    COLORREF color = RGB(0, 255, 0);
    double Frequency_1 = 1, Frequency_2 = 1, Phase = 0, Amplitude_1 = 1, Amplitude_2 = 1;
    while (!exit)
    {
        input = "";
        system("cls");
        cout << "To exit type \"e\"" << endl;
        cout << "Frequency_1 = " << Frequency_1 << endl;
        cout << "Frequency_2 = " << Frequency_2 << endl;
        cout << "Phase = " << Phase << endl;
        cout << "Amplitude_1 = " << Amplitude_1 << endl;
        cout << "Amplitude_2 = " << Amplitude_2 << endl;
        cout << "Use current frequency, phase and amplitudes? (y/n/e): ";
        cin >> input;
        if (input == "n")
        {
            cout << "What you want to change?" << endl;
            cout << "Frequencies: 1" << endl;
            cout << "Phase: 2" << endl;
            cout << "Amplitudes: 3" << endl;
            cout << "All: 4" << endl;
            cin >> input;

            if (input == "1")
            {
                cout << "Enter first frequency and second frequency in integer: ";
                cin >> Frequency_1 >> Frequency_2;
            }
            else if (input == "2")
            {
                cout << "Enter phase in degrees: ";
                cin >> Phase;
            }
            else if (input == "3")
            {
                cout << "Enter first amplitude and second amplitude: ";
                cin >> Amplitude_1 >> Amplitude_2;
            }
            else if (input == "4")
            {
                cout << "Enter first frequency, second frequency, phase and their amplitudes: ";
                cin >> Frequency_1 >> Frequency_2 >> Phase >> Amplitude_1 >> Amplitude_2;
            }
            else
            {
                cout << "Invalid input!";
            }
            if (Frequency_1 == 0)
                Frequency_1 = 1;
            if (Frequency_2 == 0)
                Frequency_2 = 1;
            if (Frequency_1 < 0)
                Frequency_1 *= -1;
            if (Frequency_2 < 0)
                Frequency_2 *= -1;
            continue;
        }
        else if (input == "y")
        {
            cout << "Static picture, animation w/o cls or animation? (1/2/3): ";
            cin >> input;
            if (input != "1" && input != "2" && input != "3")
            {
                cout << "Invalid input!";
                continue;
            }
        }
        else if (input == "e" || input == "E"){
            break;
        }
        else
        {
            cout << "Invalid input!";
            continue;
        }

        system("cls");
        GetClientRect(console, &rc);
        Sleep(5);
        if (input == "1")
        {
            Draw(hdc, rc, color, Frequency_1, Frequency_2, Phase, Amplitude_1, Amplitude_2);
        }
        else if (input == "2")
        {
            for (size_t i = 0; i < 256; i++)
            {
                Draw(hdc, rc, RGB(128, 0, i), Frequency_1, Frequency_2, Phase + (double)i*180/255, Amplitude_1, Amplitude_2);
            }
        }
        else if (input == "3")
        {
            const int trace = 2;
            for (size_t i = 0; i < 180; i++)
            {
                Draw(hdc, rc, RGB(0, 255, 0), Frequency_1, Frequency_2, Phase + i, Amplitude_1, Amplitude_2);
                Sleep(5);
                if(i >= trace) Draw(hdc, rc, RGB(12, 12, 12), Frequency_1, Frequency_2, Phase + i - trace, Amplitude_1, Amplitude_2);
            }
        }

        cin >> input;
    }

    system("cls");
    ReleaseDC(console, hdc);
    cin.ignore();

    return 0;
}

void Draw(HDC hdc, RECT rc, COLORREF color, double f1, double f2, double p, double a1, double a2)
{
    double maxAmp = (a1 > a2) ? a1 : a2;
    double minF = (f1 < f2) ? f1 : f2;
    double maxF = f1 + f2 - minF;
    int squareSize = (rc.right > rc.bottom) ? rc.bottom / 2 - 20 : rc.right / 2 - 20;
    const size_t N = (800 * maxF < 1000) ? 1000 : 800 * maxF;
    for (size_t i = 0; i < N * ((minF >= 1 && (int)maxF % (int)minF != 0) ? minF : 1); i++)
    {
        SetPixel(hdc, rc.right / 2 + squareSize * (a1 / maxAmp) * DSP::Sin(i, N / (f1 / minF)),
                 rc.bottom / 2 - squareSize * (a2 / maxAmp) * DSP::Sin(i, N / (f2 / minF), p / 360), color);
    }
}