#pragma once
#define _USE_MATH_DEFINES
#include <cmath>

namespace DSP
{
    enum Window
    {
        Rectangle,
        Triangle,
        Hamming,
        Hannah
    };

    class Filter
    {
        int Nb, Na;
        double *Kb, *Ka;
        double *Mb, *Ma;

        double RectangleWindow(int n);
        double TriangleWindow(int n);
        double HammingWindow(int n);
        double HannahWindow(int n);

    public:
        Filter(int Nb = 1, int Na = 0);
        void Average();
        // df = 2 * Bandwidth / Sampling frequency
        void LPF(double df, Window window = Rectangle);
        // df = 2 * Bandwidth / Sampling frequency
        void BPF(double df, double f0, Window window = Rectangle);
        void Clear();
        void In(double in);
        double Out();
        ~Filter();
    };
}