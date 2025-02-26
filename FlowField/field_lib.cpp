#include "field_lib.h"
#include <windows.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

namespace FlowField
{
    PointVector::PointVector(double x, double y, double dx, double dy)
    {
        this->x = x;
        this->y = y;
        this->dx = dx;
        this->dy = dy;
        this->ComputePhiAmp();
        //this->Normalize();
    }

    void PointVector::GetCoords(double &x, double &y)
    {
        x = this->x;
        y = this->y;
    }

    void PointVector::Get(double &dx, double &dy)
    {
        dx = this->dx;
        dy = this->dy;
    }

    void PointVector::GetPhiAmp(double &phi, double &amp){
        phi = this->phi;
        amp = this->amp;
    }

    void PointVector::Set(double dx, double dy)
    {
        this->dx = dx;
        this->dy = dy;
        this->ComputePhiAmp();
        //this->Normalize();
    }

    void PointVector::ComputePhiAmp(){
        if(dx > 0) phi = atan(y/x);
        else if(dx == 0){
            if(dy > 0) phi = M_PI/2;
            if(dy < 0) phi = -M_PI/2;
        } else {
            if(dy >= 0) phi = M_PI + atan(y/x);
            if(dy < 0) phi = -M_PI + atan(y/x);;
        }
            
        amp = sqrt(dx*dx + dy*dy);
    }

    void PointVector::Normalize()
    {
        double l = 1;
        if (dx != 0 && dy != 0)
            l = sqrt(dx * dx + dy * dy);
        else if (dx == 0 && dy != 0)
            l = fabs(dy);
        else if (dx != 0 && dy == 0)
            l = fabs(dx);
        dx = dx / l;
        dy = dy / l;
    }

    VectorField::VectorField(
        double X1, double Y1,
        double X2, double Y2,
        double dx, double dy)
    {
        Xf = X2 - X1;
        Yf = Y2 - Y1;
        Xc = (X2 + X1)/2;
        Yc = (Y2 + Y1)/2;
        COLS = (size_t)(Xf / dx + 1);
        ROWS = (size_t)(Yf / dy + 1);

        field = new PointVector *[ROWS];
        for (size_t i = 0; i < ROWS; i++)
        {
            field[i] = new PointVector[COLS];
            for (size_t j = 0; j < COLS; j++)
            {
                field[i][j] = PointVector(X1 + dx * (j), Y1 + dy * (i), 0, 0);
            }
        }
    }

    VectorField::~VectorField()
    {
        delete[] field;
    }

    void VectorField::setFieldFunction(PointVector (*func)(double x, double y))
    {
        fieldFunction = func;
    }

    size_t VectorField::getCols()
    {
        return COLS;
    }

    size_t VectorField::getRows()
    {
        return ROWS;
    }

    void VectorField::Compute()
    {
        double x, y;
        for (size_t i = 0; i < ROWS; i++)
        {
            for (size_t j = 0; j < COLS; j++)
            {
                field[i][j].GetCoords(x, y);
                fieldFunction(x, y).Get(x, y);
                field[i][j].Set(x, y);
            }
        }
    }

    void VectorField::DrawWords()
    {
        double x = 0, y = 0;
        for (size_t i = 0; i < ROWS; i++)
        {
            for (size_t j = 0; j < COLS; j++)
            {
                field[i][j].GetCoords(x, y);
                std::cout << i * ROWS + j << ") x = " << x << "; y = " << y << "   dx = ";
                field[i][j].Get(x, y);
                std::cout << x << "; dy = " << y << std::endl;
            }
        }
    }

    void VectorField::Draw()
    {
        HWND console = GetConsoleWindow();
        HDC hdc = GetDC(console);
        RECT rc;
        GetClientRect(console, &rc);
        double x, y, dx, dy, k;
        std::string input;
        
        k = ((rc.right / Xf) < (rc.bottom / Yf)) ? (rc.right / Xf) : (rc.bottom / Yf);
        k *= 0.95;

        system("cls");
        Sleep(25);
        for (size_t i = 0; i < ROWS; i++)
        {
            for (size_t j = 0; j < COLS; j++)
            {
                field[i][j].GetCoords(x, y);
                field[i][j].Get(dx, dy);
                /*
                for(size_t t = 0; t < 9; t++){
                    SetPixel(hdc, rc.right / 2 + k * (x - Xc) - 1 + t%3, rc.bottom / 2 - k * (y - Yc) - 1 + t/3, RGB(127 + 128 * dx, 127, 127 + 128 * dy));
                }*/
                SetPixel(hdc, rc.right / 2 + k * (x - Xc), rc.bottom / 2 - k * (y - Yc), RGB(127 + 128 * dx, 255, 127 + 128 * dy));
            }
        }
        ReleaseDC(console, hdc);
        std::cin.ignore();
    }
}