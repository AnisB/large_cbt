#pragma once

// Project includes
#include "math/types.h"

// Structure that describes a frustum plane
struct Plane
{
    float3 normal;
    float d;
};

struct Frustum
{
    Plane planes[6];
};

// Function that extracts the frustum planes from a view projection matrix
void extract_planes_from_view_projection_matrix(const float4x4 viewProj, Frustum& frustum);