#ifndef GLOBAL_CB_HLSL
#define GLOBAL_CB_HLSL

// Includes
#include "shader_lib/frustum_culling.hlsl"

// Global constant buffer
#if defined(GLOBAL_CB_BINDING_SLOT)
cbuffer GlobalCB : register(GLOBAL_CB_BINDING_SLOT)
{
    // View projection matrix
    float4x4 _ViewProjectionMatrix;

    // Inverse view projection matrix
    float4x4 _InvViewProjectionMatrix;

#if !defined(FP64_UNSUPPORTED)
    // Camera position in double as we are operating on planeraty coordinates
    double3 _CameraPosition;
#else
    float4 _UnusedGB0;
    float2 _UnusedGB1;
#endif
    // Current frame index
    uint32_t _FrameIndex;
    // Total time since the begining
    float _Time;

    // Camera position in double as we are operating on planeraty coordinates
#if !defined(FP64_UNSUPPORTED)
    float3 _CameraPositionSP;
#else
    float3 _CameraPosition;
#endif
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

#if !defined(FP64_UNSUPPORTED)
    // View projection matrix
    double4x4 _ViewProjectionMatrixDB;
#else
    float4x4 _UnusedGB2[2];
#endif
};
#endif

#if defined(GEOMETRY_CB_BINDING_SLOT)
cbuffer GeometryCB : register(GEOMETRY_CB_BINDING_SLOT)
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
#endif

#if defined(PLANET_CB_BINDING_SLOT)
cbuffer PlanetCB : register(PLANET_CB_BINDING_SLOT)
{
    // Center of the planet
    float3 _PlanetCenter;

    // Radius of the planet
    float _PlanetRadius;
};
#endif

#if defined(DEFORMATION_CB_BINDING_SLOT)
cbuffer DeformationCB : register(DEFORMATION_CB_BINDING_SLOT)
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
#endif

#if defined(UPDATE_CB_BINDING_SLOT)
cbuffer UpdateCB : register(UPDATE_CB_BINDING_SLOT)
{
    // View projection matrix used for the update
    float4x4 _UpdateViewProjectionMatrix;

    // Inverse View projection matrix used for the update
    float4x4 _UpdateInvViewProjectionMatrix;

#if !defined(FP64_UNSUPPORTED)
    // Camera position in double as we are operating on planeraty coordinates
    double3 _UpdateCameraPosition;
#else
    float4 _UnusedUB0;
    float2 _UnusedUB1;
#endif
    // Triangle size
    float _TriangleSize;
    // Max subdivision depth
    uint32_t _MaxSubdivisionDepth;

    // Camera position in float
#if !defined(FP64_UNSUPPORTED)
    float3 _UpdateCameraPositionSP;
#else
    float3 _UpdateCameraPosition;
#endif
    // FOV
    float _UpdateFOV;

    // Camera forward vector
    float3 _UpdateCameraForward;
    // Far plane
    float _UpdateFarPlaneDistance;

    // Frustum planes
    Plane _FrustumPlanes[6];
};
#endif

#if defined(MOON_CB_BINDING_SLOT)
cbuffer MoonCB : register(MOON_CB_BINDING_SLOT)
{
    // Texture size of the elevation texture
    uint2 _ElevationTextureSize;
    // Texture size of the detail texture
    uint2 _DetailTextureSize;

    // Patch sizes of the detail bands
    float _PatchSize;
    // Patch amplitude
    float _PatchAmplitude;
    // Num octaves
    int _NumOctaves;
    // Attenuation
    uint32_t _Attenuation;
};
#endif

#if defined(GLOBAL_CB_BINDING_SLOT)
bool pre_rendering_frame()
{
    return _FrameIndex == UINT32_MAX;
}
#endif

#endif // GLOBAL_CB_HLSL