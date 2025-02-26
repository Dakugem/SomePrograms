#pragma once
#define _USE_MATH_DEFINES
#include <cmath>


//TODO: Add noise function/s
namespace DSP
{
    // Initial phase = phi / (2 * pi)
    double Sin(int t, double T = 100, double phase = 0);
    double Cos(int t, double T = 100, double phase = 0);
    // Q = Pulse duration / T
    double SquarePulse(int t, int T = 100, double Q = 0.5);
    double Delta(int t);
    double Heaviside(int t);
}