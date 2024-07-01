#pragma once

// Project includes
#include "rendering/frustum.h"
#include "math/types.h"

// Project includes
#include "render_pipeline/constant_buffers.h"
#include "render_pipeline/camera.h"

struct UpdateProperties
{
    float4x4 viewProj = {};
    float4x4 invViewProj = {};
    double3 position = {};
    float3 forward = {};
    float farPlane = 0.0;
    float fov = 0.0;
    Frustum frustum = Frustum();
};

// Mirror the update properties from the camera
void mirror_update_properties(const Camera& camera, UpdateProperties& updateProperties);

// Apply the properties to the constant buffer
void set_update_properties(const UpdateProperties& updateProps, UpdateCB& updateCB);