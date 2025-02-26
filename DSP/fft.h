#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <complex>

// TODO?: Change arrays to vectors for memory control & convenience
class FFT
{
    int N_max;
    std::complex<double> *w;
    void precompute_w(int N_max);
    void fft_butterfly(std::complex<double> *data, int N, int step, bool direction);

public:
    FFT();
    static void dft(double *input, std::complex<double> *output, int N);
    static void idft(std::complex<double> *input, double *output, int N);
    // True = FFT, false = IFFT
    std::complex<double> *fft(std::complex<double> *input, int N, bool direction = true);
    ~FFT();
};