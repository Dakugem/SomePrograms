#include "field_lib.h"
#include <math.h>
#include <iostream>

using namespace std;
using namespace FlowField;

PointVector fieldFunction(double x, double y)
{
    PointVector result;
    result.Set(atan(x * y), 0);
    return result;
}

int main()
{
    const double X_min = -3.14 * 2, Y_min = -3.14 * 2, X_max = 3.14 * 2, Y_max = 3.14 * 2, dX = 0.015, dY = 0.015;
    VectorField field = VectorField(X_min, Y_min, X_max, Y_max, dX, dY);
    field.setFieldFunction(fieldFunction);

    field.Compute();
    field.Draw();

    system("pause");
    return 0;
}