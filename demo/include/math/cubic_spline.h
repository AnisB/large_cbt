#pragma

// Project includes
#include "math/types.h"
#include "math/operators.h"

// System includes
#include <algorithm>
#include <vector>

// Undef the max/min macros
#undef max
#undef min

// Helpers
void spline_weights(float u, float weights[4]);
void spline_weights(double u, double weights[4]);

template<typename T, typename P>
T evaluate_catmull_rom_spline(const std::vector<T>& splinePoints, P t, bool loop)
{
    // First we find which segment we'll be choosing
    P tP = t * (splinePoints.size());

    int32_t segmentID = (int32_t)tP;
    P remappedT = tP - (P)segmentID;

    // Grab the 4 points we need
    const T& p0 = splinePoints[loop ? (segmentID - 1 + splinePoints.size()) % splinePoints.size() : std::max(segmentID - 1, 0)];
    const T& p1 = splinePoints[std::max(segmentID, 0)];
    const T& p2 = splinePoints[loop ? (segmentID + 1) % splinePoints.size() : std::min(segmentID + 1, int32_t(splinePoints.size()) - 1)];
    const T& p3 = splinePoints[loop ? (segmentID + 2) % splinePoints.size() : std::min(segmentID + 2, int32_t(splinePoints.size()) - 1)];

    // Evaluate the weights
    P weights[4];
    spline_weights(remappedT, weights);

    // Combine
    return p0 * weights[0] + p1 * weights[1] + p2 * weights[2] + p3 * weights[3];
}