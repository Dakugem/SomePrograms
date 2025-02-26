#include "signal_source.h"

namespace DSP
{
    double Sin(int t, double T, double phase)
    {
        return sin(2 * M_PI * ((double)t / T + phase));
    }

    double Cos(int t, double T, double phase)
    {
        return cos(2 * M_PI * ((double)t / T + phase));
    }

    double SquarePulse(int t, int T, double Q)
    {
        return (t % T < Q * T) ? 1 : 0;
    }

    double Delta(int t)
    {
        return (t == 0) ? 1 : 0;
    }

    double Heaviside(int t)
    {
        return (t == 0) ? 0 : 1;
    }
}