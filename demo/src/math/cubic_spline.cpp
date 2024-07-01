// Project includes
#include "math/cubic_spline.h"

void spline_weights(float u, float weights[4])
{
    float T = u, S = 1.0f - u;
    weights[0] = (S * S * S) / 6.0f;
    weights[1] = ((4.0f * S * S * S + T * T * T) + (12.0f * S * T * S + 6.0f * T * S * T)) / 6.0f;
    weights[2] = ((4.0f * T * T * T + S * S * S) + (12.0f * T * S * T + 6.0f * S * T * S)) / 6.0f;
    weights[3] = (T * T * T) / 6.0f;
}

void spline_weights(double u, double weights[4])
{
    double T = u, S = 1.0 - u;
    weights[0] = (S * S * S) / 6.0;
    weights[1] = ((4.0 * S * S * S + T * T * T) + (12.0 * S * T * S + 6.0 * T * S * T)) / 6.0;
    weights[2] = ((4.0 * T * T * T + S * S * S) + (12.0 * T * S * T + 6.0 * S * T * S)) / 6.0;
    weights[3] = (T * T * T) / 6.0;
}