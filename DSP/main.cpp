#include "signal_source.h"
#include "filter.h"
#include "fft.h"
#include <windows.h>
#include <iostream>

using namespace std;
using namespace DSP;
#define cin std::cin // ��-�� <complex.h>

void drawArray(HDC hdc, int x, double *arr, int N);
void draw(HDC, int, double, int, COLORREF = RGB(0, 255, 0));
int main()
{
    Filter filter1 = Filter(50);
    filter1.Average();
    Filter filter2 = Filter(49);
    filter2.LPF(0.1);
    HWND console = GetConsoleWindow();
    RECT rc;
    HDC hdc = GetDC(console);
    COLORREF color = RGB(0, 255, 0);

    int k = 2;
    double **time = new double*[k];
    complex<double> **frq = new complex<double>*[k];
    while (true)
    {
        GetClientRect(console, &rc);
        filter1.Clear();
        filter2.Clear();
        int N = 9;
        double *signals = new double[N];
        
        for(int i = 0; i < k; i++) time[i] = new double[rc.right - rc.left];
        for(int i = 0; i < k; i++) frq[i] = new complex<double>[rc.right - rc.left];

        for (int x = rc.left; x < rc.right; x++)
        {
            signals[0] = Sin(x, rc.right / 40) * (Sin(x, rc.right / 1)
             + 0.4 * Sin(x, rc.right / 5) + 0.1 * Sin(x, rc.right / 19)) / 1.5;
            signals[1] = SquarePulse(x, 100);
            filter1.In(signals[1]);
            signals[2] = filter1.Out();
            signals[3] = Sin(x, rc.right / 20) * exp(-0.001 * x); 
            signals[4] = Sin(x, (1 + x / (1000. - 200 * Sin(x, 100))) * rc.right / 20.);
            signals[5] = Sin(x, (1 + 0.1 * Sin(x, 250)) * rc.right / 20.);
            signals[6] = 2 * Sin(x, 20) * (signals[1] - 0.5);
            signals[7] = Sin(x, 400)*Sin(x, 20);//(Sin(x, 50) + Sin(x, 5)) / 2;
            time[0][x] = signals[1];
            filter2.In(signals[7]);
            signals[8] = (signals[7] > 0)? signals[7] : 0;//filter2.Out();
            time[1][x] = signals[8];

            //drawArray(hdc, x, signals, N);

            draw(hdc, x, signals[0], 1);
            draw(hdc, x, signals[1], 2);
            draw(hdc, x, signals[2], 3);
            draw(hdc, x, signals[3], 4);
            draw(hdc, x, signals[4], 5);
            draw(hdc, x, signals[5], 6);
            draw(hdc, x, signals[6], 7);
            draw(hdc, x, signals[7], 8);
            draw(hdc, x, signals[8], 10);
        }
        FFT::dft(time[0], frq[0], rc.right - rc.left);
        FFT::dft(time[1], frq[1], rc.right - rc.left);
        //FFT::idft(frq[1], time[1], rc.right - rc.left);

        for (int x = rc.left; x < rc.right; x++)
        {
            draw(hdc, x, abs(frq[0][x])*20/(rc.right), 9);
            draw(hdc, x, abs(frq[1][x])*20/(rc.right), 11);
        }

        delete[] signals;
        for(int i = 0; i < 2; i++) delete[] time[i], frq[i];
    }

    delete[] time, frq;
    ReleaseDC(console, hdc);
    cin.ignore();

    return 0;
}

void drawArray(HDC hdc, int x, double *arr, int N)
{
    for(int i = 0; i < N; i++){
        SetPixel(hdc, x, -arr[i] * 20 + 50 * (i + 0.5), RGB(0, 255, 0));
    }
}

void draw(HDC hdc, int x, double y, int shift, COLORREF color)
{
    SetPixel(hdc, x, -y * 20 + 50 * (shift - 0.5), color);
}