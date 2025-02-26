#include "filter.h"
#include <iostream>

namespace DSP
{
    double Filter::RectangleWindow(int n) { return 1; }
    double Filter::TriangleWindow(int n)
    {
        // return 1 - (double)(2 * abs(n - Nb / 2)) / (Nb);
        return 1 - (double)(2 * abs(n)) / (Nb * 2);
    }
    double Filter::HammingWindow(int n)
    {
        // return 0.54 - 0.46*cos(2*M_PI*n/Nb);
        return 0.54 - 0.46 * cos(2 * M_PI * (n + Nb) / (2 * Nb));
    }
    double Filter::HannahWindow(int n)
    {
        return 0.5 * (1 - cos(2 * M_PI * n / Nb));
    }

    Filter::Filter(int Nb, int Na)
    {
        this->Nb = Nb;
        this->Na = Na + 1;
        Kb = new double[this->Nb];
        Ka = new double[this->Na];
        Mb = new double[this->Nb];
        Ma = new double[this->Na];

        for (int i = 0; i < this->Nb; i++)
            Mb[i] = 0;
        for (int i = 0; i < this->Na; i++)
            Ma[i] = 0;
    }

    void Filter::Average()
    {
        for (int i = 0; i < Nb; i++)
        {
            Kb[i] = (double)1 / Nb;
        }
    }

    // Window functions must have the same axis of symmetry as the impulse characteristic
    // Idnw but truncated impulse characteristic appear better than not truncated
    void Filter::LPF(double df, Window window)
    {
        double (DSP::Filter::*windowFunc)(int);
        switch (window)
        {
        case Rectangle:
            windowFunc = RectangleWindow;
            break;
        case Triangle:
            windowFunc = TriangleWindow;
            break;
        case Hamming:
            windowFunc = HammingWindow;
            break;
        case Hannah:
            windowFunc = HannahWindow;
            break;
        default:
            windowFunc = RectangleWindow;
            break;
        }

        /*
        for(int i = 0; i < Nb; i++){
            if(i != Nb/2) Kb[i] = (2 / M_PI) * sin(M_PI * df * (i - Nb/2)) / (i - Nb/2);
            else Kb[i] = 2 * df;
            Kb[i] *= (this->*windowFunc)(i);
        }*/

        Kb[0] = 2 * df * (this->*windowFunc)(0);
        for (int i = 1; i < Nb; i++)
        {
            Kb[i] = (this->*windowFunc)(i) * (2 / M_PI) * sin(M_PI * df * i) / i;
        }
    }

    void Filter::BPF(double df, double f0, Window window)
    {
        this->LPF(df, window);

        for (int i = 0; i < Nb; i++)
        {
            Kb[i] *= 2 * cos(2 * M_PI * f0 * i);
        }
    }

    void Filter::Clear()
    {
        for (int i = 0; i < Na; i++)
            Ma[i] = 0;
        for (int i = 0; i < Nb; i++)
            Mb[i] = 0;
    }

    void Filter::In(double in)
    {
        double res = 0;

        for (int i = Nb - 1; i > 0; i--)
        {
            Mb[i] = Mb[i - 1];
        }
        Mb[0] = in;

        for (int i = 0; i < Nb; i++)
        {
            res += Mb[i] * Kb[i];
        }

        for (int i = Na - 1; i > 0; i--)
        {
            Ma[i] = Ma[i - 1];
        }

        for (int i = 1; i < Na; i++)
        {
            res -= Ma[i] * Ka[i];
        }
        Ma[0] = res;
    }

    double Filter::Out()
    {
        return Ma[0];
    }

    Filter::~Filter()
    {
        delete[] Kb, Ka, Mb, Ma;
    }
}