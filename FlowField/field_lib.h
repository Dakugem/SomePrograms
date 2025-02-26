#include <cstddef>
#pragma once

namespace FlowField
{
    class PointVector
    {
        double x, y, dx, dy;
        double phi, amp;

    public:
        PointVector(double x = 0, double y = 0, double dx = 0, double dy = 0);
        void GetCoords(double &x, double &y);
        void Get(double &dx, double &dy);
        void GetPhiAmp(double &phi, double &amp);
        void Set(double dx, double dy);
        void ComputePhiAmp();
        // Maybe not use Normalize(), to show magnitude by color.
        void Normalize();
    };

    class VectorField
    {
        size_t COLS, ROWS;
        double Xf, Yf, Xc, Yc, Xd, Yd;
        PointVector **field;
        PointVector (*fieldFunction)(double x, double y);
    public:
        VectorField(
            double X1 = -1, double Y1 = -1,
            double X2 = 1, double Y2 = 1,
            double dx = 0.1, double dy = 0.1);
        ~VectorField();

        void setFieldFunction(PointVector (*func)(double x, double y));
        size_t getCols();
        size_t getRows();
        //fieldFunction may be a zero pointer!!!
        void Compute();
        void DrawWords();
        void Draw();
    };
}