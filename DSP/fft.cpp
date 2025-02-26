#include "fft.h"
#include <iostream>

FFT::FFT(){ N_max = 0;}

void FFT::precompute_w(int N_max){
    if (this->N_max != 0) {
        delete[] w;
    }

    this->N_max = N_max;
    w = new std::complex<double>[N_max];
    for (int i = 0; i < N_max / 2; ++i) {
        w[i] = exp(std::complex<double>(0, -2.0 * M_PI * i / N_max));
    }
}

void FFT::fft_butterfly(std::complex<double>* data, int N, int step, bool direction) {
    if (N <= 1) return;

    int half = N / 2;
    std::complex<double>* even = new std::complex<double>[half];
    std::complex<double>* odd = new std::complex<double>[half];

    for (int i = 0; i < half; ++i) {
        even[i] = data[i * 2];
        odd[i] = data[i * 2 + 1];
    }

    fft_butterfly(even, half, step * 2, direction);
    fft_butterfly(odd, half, step * 2, direction);

    for (int i = 0; i < half; ++i) {
        std::complex<double> t = odd[i]
        * ((!direction)? conj(w[i * step]) : w[i * step]);
        data[i] = even[i] + t;
        data[i + half] = even[i] - t;
    }

    delete[] even;
    delete[] odd;
}

std::complex<double>* FFT::fft(std::complex<double>* input, int N, bool direction) {
    if (N <= 1) return nullptr;
    if (N > N_max) precompute_w(N);

    std::complex<double>* data = new std::complex<double>[N];
    for(int i = 0; i < N; ++i){
        data[i] = input[i];
    }

    fft_butterfly(data, N, N_max/N, direction);

    if(!direction){
        for(int i = 0; i < N; i++) data[i] /= N;
    }

    return data;
}

FFT::~FFT(){ delete[] w;}

void FFT::dft(double* input, std::complex<double> *output, int N){
    //const std::complex<double> w_1(cos(2*M_PI/N), -sin(2*M_PI/N));
    const std::complex<double> w_1(exp(std::complex<double>(0, -2*M_PI/N)));
    std::complex<double> step, w;
    
    for(int n = 0; n < N; ++n){
        std::complex<double> temp(0,0);
        step = pow(w_1, n);
        w = step;

        temp += input[0];
        for(int k = 1; k < N; ++k){
            temp += input[k]*w;
            w *= step;
        }

        //if(abs(temp.real()) < 1e-15) temp.real(0);
        //if(abs(temp.imag()) < 1e-15) temp.imag(0);
        output[n] = temp;
    }
}

void FFT::idft(std::complex<double>* input, double *output, int N){
    const std::complex<double> w_1(cos(2*M_PI/N), sin(2*M_PI/N));
    std::complex<double> step, w;

    for(int k = 0; k < N; ++k){
        std::complex<double> temp(0,0);
        step = pow(w_1, k);
        w = step;

        temp += input[0];
        for(int n = 1; n < N; ++n){
            temp += input[n]*w;
            w *= step;
        }

        //if(abs(temp.real()) < 1e-15) temp.real(0);
        //if(abs(temp.imag()) < 1e-15) temp.imag(0);
        output[k] = temp.real()/N;
    }
}