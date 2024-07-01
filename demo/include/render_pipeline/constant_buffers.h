#pragma once

// Project includes
#include <rendering/frustum.h>

// Global constant buffer
struct GlobalCB
{
    // View projection matrix
    float4x4 _ViewProjectionMatrix;

    // Inverse view projection matrix
    float4x4 _InvViewProjectionMatrix;

    // Camera position in double as we are operating on planeraty coordinates
    double3 _CameraPosition;
    // Current frame index
    uint32_t _FrameIndex;
    // Total time since the begining
    float _Time;

    // Camera position in double as we are operating on planeraty coordinates
    float3 _CameraPositionSP;
    float _CullFlag;

    // Screen size and inverse screen size
    float4 _ScreenSize;

    // Sun elevation
    float3 _SunDirection;
    // Field of view
    float _FoV;

    // Wireframe color
    float3 _WireFrameColor;
    // Wireframe size
    float _WireFrameSize;

    // Padding
    float2 _PaddingGB0;
    // Flag for the screen space shadow
    float _ScreenSpaceShadow;
    // Far plane distance
    float _FarPlaneDistance;

    // View projection matrix
    double4x4 _ViewProjectionMatrixDB;
};

struct UpdateCB
{
    // View projection matrix used for the update
    float4x4 _UpdateViewProjectionMatrix;

    // Inverse View projection matrix used for the update
    float4x4 _UpdateInvViewProjectionMatrix;

    // Camera position in double as we are operating on planeraty coordinates
    double3 _UpdateCameraPosition;
    // Triangle size
    float _TriangleSize;
    // Max subdivision depth
    uint32_t _MaxSubdivisionDepth;

    // Camera position in float
    float3 _UpdateCameraPositionSP;
    // FOV
    float _UpdateFOV;

    // Camera forward vector
    float3 _UpdateCameraForward;
    // Far plane
    float _UpdateFarPlaneDistance;

    // Frustum planes
    Plane _UpdateFrustumPlanes[6];
};

struct GeometryCB
{
    // Total number of elements of the geometry
    uint32_t _TotalNumElements;
    // Base depth of the planet mesh
    uint32_t _BaseDepth;
    // Total Num Vertices
    uint32_t _TotalNumVertices;
    // Material ID
    uint32_t _MaterialID;
};

struct PlanetCB
{
    // Center of the planet
    float3 _PlanetCenter;

    // Radius of the planet
    float _PlanetRadius;
};

struct DeformationCB
{
    // Patch Size for the deformation
    float4 _PatchSize;

    // Patch roughness
    float4 _PatchRoughness;
    
    // Choppiness of the displacement
    float _Choppiness;
    // Is the attenuation enabled?
    int _Attenuation;
    // Amplification
    float _Amplification;
    // Patch Flags
    uint32_t _PatchFlags;
};

struct WaterSimulationCB
{
    // Resolution at which the simulation is evaluated
    uint32_t _SimulationRes;
    // Time at which the simulation should be evaluated
    float _SimulationTime;
    // Choppiness
    float _Choppiness;
    // Amplification
    float _Amplification;

    // Individual sizes of the wave bands
    float4 _PatchSize;

    // Horizontal wind direction
    float4 _PatchWindOrientation;

    // Controls how much the wind affect the current of the waves
    float4 _PatchDirectionDampener;

    // Wind speed per band
    float4 _PatchWindSpeed;
};

struct MoonCB
{
    // Texture size of the elevation texture
    uint2 _ElevationTextureSize;
    // Texture size of the detail texture
    uint2 _DetailTextureSize;

    // Patch sizes of the detail bands
    float _PatchSize;
    // Patch amplitudes
    float _PatchAmplitude;
    // Num octaves
    int32_t _NumOctaves;
    // Attenuation
    uint32_t _Attenuation;
};