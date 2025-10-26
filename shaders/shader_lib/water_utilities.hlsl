#ifndef WATER_UTILITIES_H
#define WATER_UTILITIES_H

// Includes
#include "shader_lib/common.hlsl"

// Water constants
#define ONE_OVER_SQRT2 0.70710678118
#define EARTH_GRAVITY 9.81
#define PHILLIPS_AMPLITUDE_SCALAR 0.2
#define NOISE_FUNCTION_OFFSET 64

// Water CB
cbuffer WaterSimulationCB : register(WATER_SIMULATION_CB_SLOT)
{
    // Resolution at which the simulation is evaluated
    uint _SimulationRes;
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

    // Roughness per patch
    float4 _PatchRoughness;
};

float phillips_spectrum(float2 k, float2 w, float V, float directionDampener, float patchSize)
{
    float kk = k.x * k.x + k.y * k.y;
    float result = 0.0;
    if (kk != 0.0)
    {
        float L = (V * V) / EARTH_GRAVITY;
        // To avoid _any_ directional bias when there is no wind we lerp towards 0.5f
        float wk = lerp(dot(normalize(k), w), 0.5, directionDampener);
        float phillips = (exp(-1.0f / (kk * L * L)) / (kk * kk)) * (wk * wk);
        result = phillips * (wk < 0.0f ? directionDampener : 1.0);
    }
    return PHILLIPS_AMPLITUDE_SCALAR * result / (patchSize * patchSize);
}

float2 complex_conjugate(float2 a)
{
    return float2(a.x, -a.y);
}

float2 complex_exp(float arg)
{
    return float2(cos(arg), sin(arg));
}

float2 complex_mult(float2 a, float2 b)
{
    return float2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

float frquency_phase(float2 complex)
{
    return atan(complex.y / complex.x);
}

float frequency_amplitude(float2 complex)
{
    return sqrt(complex.x * complex.x + complex.y * complex.y);
}

float3 ShuffleDisplacement(float3 displacement)
{
    return float3(-displacement.y, displacement.x, -displacement.z);
}

void EvaluateDisplacedPoints(float3 center, float3 right, float3 up, float pixelSize, out float3 p0, out float3 p1, out float3 p2)
{
    p0 = float3(center.x, center.y, center.z);
    p1 = float3(right.x, right.y, right.z) + float3(pixelSize, 0, 0);
    p2 = float3(up.x, up.y, up.z) + float3(0, 0, pixelSize);
}

#endif // WATER_UTILITIES_H
