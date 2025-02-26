#include "signal_source.h"
#include "fft.h"
#include <windows.h>
#include <iostream>

using namespace std;
using namespace DSP;
#define cin std::cin
#define N 256

void draw(HDC, int, int, int, COLORREF);
int main(){
    HWND console = GetConsoleWindow();
    RECT rc;
    HDC hdc = GetDC(console);
    COLORREF color = RGB(0, 255, 0);

    FFT fft;
    std::complex<double>* input = new complex<double>[N];
    std::complex<double> *output;

    int x = 0, fp;
    while(true){
        GetClientRect(console, &rc);
        if(x >= rc.right) x = 0;

        for(int i = 0; i < N; ++i){
            input[i] = 0;

            //input[i] = Sin(i, N/4) + Sin(i, N) + Sin(i, N/32); 
            input[i] += Sin(i, N / (1 + (N / 4 - 1) * (double)x / rc.right));
            input[i] += Sin(i, N / (32 + (N / 8 - 1) * Sin(x, rc.right/4)));
            //input[i] /= 2;//Normalization?

            input[i] += 0;// Noise add here
        }

        output = fft.fft(input, N);

        for(int i = 0; i < N; ++i){
            COLORREF color = RGB(127, 127, 127);
            fp = 3 * abs(output[i]) * 2 / N;
            if(fp >= 0 && fp <= 1)      color = RGB(0, 255 * fp, 0);
            else if(fp > 1 && fp <= 2)  color = RGB(0, 0, 255 * (fp - 1));
            else if(fp > 2 && fp <= 3)  color = RGB(255 * (fp - 2), 0, 0);
            else                        color = RGB(255, 255, 255);
            draw(hdc, x, i + 1, 0, color);
        }

        x++;
    }

    delete[] input, output;
    ReleaseDC(console, hdc);
    cin.ignore();

    return 0;
}

void draw(HDC hdc, int x, int y, int shift, COLORREF color)
{
    SetPixel(hdc, x, -y + N * (shift + 1), color);
}