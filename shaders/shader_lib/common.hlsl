#ifndef COMMON_H
#define COMMON_H

// Generic constants
#define PI 3.14159265358979323846
#define HALF_PI (PI / 2.0)
#define TWO_PI (PI * 2.0)
#define FLT_EPSILON 1.192092896e-07
#define INV_PI (1.0 /  PI)
#define FLT_MIN 1.175494351e-38
#define FLT_MAX 3.402823466e+38
#define PHI 1.61803398874989484820459
#define UINT32_MAX 0xffffffffU

// Set of materials
#define UNUSED_MATERIAL 0
#define EARTH_MATERIAL 1
#define MOON_MATERIAL 2

// Included after the constants
#if !defined(FP64_UNSUPPORTED)
    #include "shader_lib/double_math.hlsl"
    #define REAL3_DP double3
    #define REAL2_DP double2
    #define REAL_DP double
#else
    #define REAL3_DP float3
    #define REAL2_DP float2
    #define REAL_DP float
#endif

// Binding slot helpers
#define CBV_SLOT(NUM_SLOT) b##NUM_SLOT
#define SRV_SLOT(NUM_SLOT) t##NUM_SLOT
#define UAV_SLOT(NUM_SLOT) u##NUM_SLOT
#define SPL_SLOT(NUM_SLOT) s##NUM_SLOT

// Unfortunately this breaks on vulkan, so I am doing it manually.
uint2 reversebits_uint2(const uint2 input)
{
    uint2 x = input;
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return((x >> 16) | (x << 16));
}

// http://www.dspguide.com/ch2/6.htm
float gaussian_distribution(float u, float v)
{
    return sqrt(-2.0 * log(max(u, 1e-6f))) * cos(PI * v);
}

float pick_closest(float p, float n, float s)
{
    float nC = n + s;
    float distX0 = p - n;
    float distX1 = p - nC;
    return abs(distX0) < abs(distX1) ? n : nC;
}

float2 compare_and_pick(float2 p, float2 n, float s)
{
    return float2(pick_closest(p.x, n.x, s), pick_closest(p.y, n.y, s));
}

void evaluate_frac_derivatives(float2 bandUV, out float2 uvDDX, out float2 uvDDY)
{
    // Evaluate the derivatives
    float2 ddxUV = ddx(bandUV);
    float2 uvX = bandUV + ddxUV;
    uvX = compare_and_pick(bandUV, uvX, 1.0);
    uvX = compare_and_pick(bandUV, uvX, -1.0);
    uvDDX = bandUV - uvX;

    float2 ddyUV = ddy(bandUV);
    float2 uvY = bandUV + ddyUV;
    uvY = compare_and_pick(bandUV, uvY, 1.0);
    uvY = compare_and_pick(bandUV, uvY, -1.0);
    uvDDY = bandUV - uvY;
}

int2 repeat_coord(int2 tap, uint width, uint height)
{
    return int2(uint(tap.x) % width, uint(tap.y) % height);
}

float3 surface_gradient_from_perturbed_normal(float3 nrmVertexNormal, float3 v)
{
    float3 n = nrmVertexNormal;
    float s = 1.0 / max(FLT_EPSILON, abs(dot(n, v)));
    return s * (dot(n, v) * n - v);
}

uint4 hash_function_uint4(uint3 coord)
{
    uint4 x = coord.xyzz;
    x = ((x >> 16u) ^ x.yzxy) * 0x45d9f3bu;
    x = ((x >> 16u) ^ x.yzxz) * 0x45d9f3bu;
    x = ((x >> 16u) ^ x.yzxx) * 0x45d9f3bu;
    return x;
}

float4 hash_function_float4(uint3 p)
{
    return hash_function_uint4(p) / (float)0xffffffffU;
}

float sanitize_normal(inout float3 normalWS, float3 viewWS, float3 positionRWS)
{
    // Check if we need to patch the normal
    float NdotV = dot(normalWS, viewWS);
    // the geometry normal is probably flipped, reverse it
    if (NdotV < 0.0)
    {
        normalWS =  reflect(normalWS, viewWS);
        NdotV = -NdotV;
    }
    return NdotV;
}

// - r0: ray origin
// - rd: normalized ray direction
// - s0: sphere center
// - sR: sphere radius
// - Returns distance from r0 to first intersecion with sphere,
//   or -1.0 if no intersection.
float ray_sphere_intersect_nearest(float3 r0, float3 rd, float3 s0, float sR)
{
    float a = dot(rd, rd);
    float3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sR * sR);
    float delta = b * b - 4.0*a*c;
    if (delta < 0.0 || a == 0.0)
        return -1.0;
    float sol0 = (-b - sqrt(delta)) / (2.0*a);
    float sol1 = (-b + sqrt(delta)) / (2.0*a);
    if (sol0 < 0.0 && sol1 < 0.0)
        return -1.0;
    if (sol0 < 0.0)
        return max(0.0, sol1);
    else if (sol1 < 0.0)
        return max(0.0, sol0);
    return max(0.0, min(sol0, sol1));
}

#if !defined(FP64_UNSUPPORTED)
double ray_sphere_intersect_nearest_d(double3 r0, double3 rd, double3 s0, double sR)
{
    double a = dot_double(rd, rd);
    double3 s0_r0 = r0 - s0;
    double b = 2.0 * dot_double(rd, s0_r0);
    double c = dot_double(s0_r0, s0_r0) - (sR * sR);
    double delta = b * b - 4.0*a*c;
    if (delta < 0.0 || a == 0.0)
        return -1.0;
    double sol0 = (-b - sqrt_double(delta)) / (2.0*a);
    double sol1 = (-b + sqrt_double(delta)) / (2.0*a);
    if (sol0 < 0.0 && sol1 < 0.0)
        return -1.0;
    if (sol0 < 0.0)
        return max(0.0, sol1);
    else if (sol1 < 0.0)
        return max(0.0, sol0);
    return max(0.0, min(sol0, sol1));
}
#endif

float3 apply_wireframe(float3 color, float3 wireFrameColor, float wireframeSize, float3 dist)
{
    if (wireframeSize > 0.0)
    {
        float3 d2 = dist * dist;
        float nearest = min(min(d2.x, d2.y), d2.z);
        float f = exp2(-nearest / wireframeSize);
        color = lerp(color, wireFrameColor, f);
    }
    return color;
}

// Includes
#include "shader_lib/sampling.hlsl"
#include "shader_lib/color.hlsl"
#include "shader_lib/transform.hlsl"

#endif
