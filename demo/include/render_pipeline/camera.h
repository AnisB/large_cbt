#pragma once

// Project includes
#include "math/types.h"

struct Camera
{
    // Projection parameters
    float fov;
    float2 nearFar;
    float aspectRatio;

    // View parameters
    double3 position;
    float3 angles;

    // Zoom parameters
    float3 scaleOffset;

    // Transformation matrices
    float4x4 view;
    float4x4 projection;
    double4x4 projectionDB;

    // Compound matrices
    float4x4 viewProjection;
    double4x4 viewProjectionDB;
    float4x4 invViewProjection;
};
