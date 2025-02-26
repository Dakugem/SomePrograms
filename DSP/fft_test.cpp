#include "signal_source.h"
#include "filter.h"
#include "fft.h"
#include <iostream>
#include <windows.h>
#include <ctime>

using namespace std;
using namespace DSP;

template<class T>
void show(T *arr, int N);
int main(){
    unsigned int begin;
    unsigned int end;
    HWND console = GetConsoleWindow();
    RECT rc;
    HDC hdc = GetDC(console);
    COLORREF color = RGB(0, 255, 0);
    const int N = 1024;
    double df = 0.2;
    Filter filter(199);
    filter.LPF(df, Hamming);
    //filter.BPF(df, 0.2, Hamming);
    std::complex<double>* array = new complex<double>[N];
    std::complex<double>* array2 = new complex<double>[N];
    std::complex<double>* arraysinfilter = new complex<double>[N];
    std::complex<double>* fftFirst = new complex<double>[N];

    std::complex<double> *res, *res2, *resfilter, *fftResult, *ifftResult;

    for(int i = 0; i < N; i++){
        double temp = 0;
        for(int j = 0; j < N/2; j++) temp += sin(2*M_PI*i*j/N);
        temp /= N/2;

        filter.In(temp);
        array[i] = filter.Out();
    }

    for(int i = 0; i < N; i++){
        fftFirst[i] = sin(1024*2*M_PI*i/(double)N);
    }

    FFT fft;
    begin = clock();
    fftResult = fft.fft(fftFirst, N);
    end = clock();
    //cout << "FFT time for " << N << " elements: " << end - begin << endl;

    begin = clock();
    ifftResult = fft.fft(fftResult, N, false);
    end = clock();
    //cout << "IFFT time for " << N << " elements: " << end - begin << endl;

    begin = clock();
    res = fft.fft(array, N);
    end = clock();
    //cout << "FFT time for " << N << " elements: " << end - begin << endl;
    begin = clock();
    resfilter = fft.fft(arraysinfilter, N);
    end = clock();
    //cout << "FFT time for " << N << " elements: " << end - begin << endl;
    
    begin = clock();
    array2 = fft.fft(res, N, false);
    end = clock();
    //cout << "IFFT time for " << N << " elements: " << end - begin << endl;
    
    while(true){
        int k = 1;
        for(int i = 0; i < N*k; i++){
            SetPixel(hdc, i, -array[i/k].real()*20 + 50, color);
            SetPixel(hdc, i, -20 + 100, RGB(255, 0, 0));
            SetPixel(hdc, i, 100, RGB(255, 0, 0));
            SetPixel(hdc, i, -abs(res[i/k])*20 + 100, color);
            //SetPixel(hdc, i, -arg(res[i/k])*20 + 150, color);
            SetPixel(hdc, i, -array2[i/k].real()*20 + 150, color);

            SetPixel(hdc, i, -fftFirst[i/k].real()*20 + 475, color);
            SetPixel(hdc, i, -abs(fftResult[i/k])*20*2/N + 525, color);
            SetPixel(hdc, i, -ifftResult[i/k].real()*20 + 575, color);
        }

        for(int i = 70; i <= 100; i++){
            SetPixel(hdc, df*N*k/2, i, RGB(255, 0, 0));
            SetPixel(hdc, (2-df)*N*k/2, i, RGB(255, 0, 0));
        }
        
    }

    //show(array, 10);
    //show(array, 10);
    //show(array, 5);
    //show(array + 2, 4);
    delete[] array, array2, arraysinfilter, fftFirst, res, res2,
    resfilter, fftResult, ifftResult;
    std::cin.ignore();
    return 0;
}

template<class T>
void show(T *arr, int N){
    cout << "SHOW ARRAY" << endl;
    for(int i = 0; i < N; i++){
        cout << arr[i] << endl;
    }
    cout << endl;
}