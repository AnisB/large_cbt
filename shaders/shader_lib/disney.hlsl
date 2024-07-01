#ifndef DISNEY_HLSL
#define DISNEY_HLSL

// Includes
#include "shader_lib/bsdf.hlsl"

float F_Schlick(float f0, float f90, float u)
{
    float x = 1.0 - u;
    float x2 = x * x;
    float x5 = x * x2 * x2;
    return (f90 - f0) * x5 + f0;
}

float F_Schlick(float f0, float u)
{
    return F_Schlick(f0, 1.0, u);
}

float Sq(float v)
{
    return v*v;
}

// Ref: Steve McAuley - Energy-Conserving Wrapped Diffuse
float ComputeWrappedDiffuseLighting(float NdotL, float w)
{
    return saturate((NdotL + w) / ((1.0 + w) * (1.0 + w)));
}

// Inline D_GGX() * V_SmithJointGGX() together for better code generation.
float DV_SmithJointGGX(float NdotH, float NdotL, float NdotV, float roughness, float partLambdaV)
{
    float a2 = Sq(roughness);
    float s = (NdotH * a2 - NdotH) * NdotH + 1.0;

    float lambdaV = NdotL * partLambdaV;
    float lambdaL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);

    float2 D = float2(a2, s * s);            // Fraction without the multiplier (1/Pi)
    float2 G = float2(1, lambdaV + lambdaL); // Fraction without the multiplier (1/2)

    // This function is only used for direct lighting.
    // If roughness is 0, the probability of hitting a punctual or directional light is also 0.
    // Therefore, we return 0. The most efficient way to do it is with a max().
    return INV_PI * 0.5 * (D.x * G.x) / max(D.y * G.y, FLT_MIN);
}

float SmithJointGGXPartLambdaVApprox(float NdotV, float roughness)
{
    return NdotV * (1 - roughness) + roughness;
}

float DisneyDiffuseNoPI(float NdotV, float NdotL, float LdotV, float perceptualRoughness)
{
    // (2 * LdotH * LdotH) = 1 + LdotV
    // float fd90 = 0.5 + (2 * LdotH * LdotH) * perceptualRoughness;
    float fd90 = 0.5 + (perceptualRoughness + perceptualRoughness * LdotV);
    // Two schlick fresnel term
    float lightScatter = F_Schlick(1.0, fd90, NdotL);
    float viewScatter = F_Schlick(1.0, fd90, NdotV);

    // Normalize the BRDF for polar view angles of up to (Pi/4).
    // We use the worst case of (roughness = albedo = 1), and, for each view angle,
    // integrate (brdf * cos(theta_light)) over all light directions.
    // The resulting value is for (theta_view = 0), which is actually a little bit larger
    // than the value of the integral for (theta_view = Pi/4).
    // Hopefully, the compiler folds the constant together with (1/Pi).
    return rcp(1.03571) * (lightScatter * viewScatter);
}

float DisneyDiffuse(float NdotV, float NdotL, float LdotV, float perceptualRoughness)
{
    return INV_PI * DisneyDiffuseNoPI(NdotV, NdotL, LdotV, perceptualRoughness);
}

float3 EvaluateBSDF(BSDFData bsdfData, float NdotV, float3 lightDir)
{
    // Pre-lighting
    float NdotL = dot(bsdfData.normalWS, lightDir);
    float LdotV = dot(lightDir, bsdfData.viewWS);
    float invLenLV = rsqrt(max(2.0 * LdotV + 2.0, 0.0000001));    // invLenLV = rcp(length(L + V)), clamp to avoid rsqrt(0) = inf, inf * 0 = NaN
    float NdotH = saturate((NdotL + NdotV) * invLenLV);
    float LdotH = saturate(invLenLV * LdotV + invLenLV);
    float partLambdaV = SmithJointGGXPartLambdaVApprox(NdotV, bsdfData.roughness);
    float clampedNdotL = saturate(NdotL);
    float F = F_Schlick(0.02, LdotH);
    float NdotLWrappedDiffuseLowFrequency = ComputeWrappedDiffuseLighting(NdotL, 0.5);

    // Diffuse lighting
#if defined(ENABLE_WRAP_LIGHTING)
    float3 diffR = (1 - F) * lerp(1.f, NdotLWrappedDiffuseLowFrequency, 1.0f) * bsdfData.diffuseColor;
#else
    float3 diffR = DisneyDiffuse(NdotV, NdotL, LdotV, sqrt(bsdfData.roughness)) * bsdfData.diffuseColor * clampedNdotL;
#endif

    // Specular lighting
    float3 specR = F * (DV_SmithJointGGX(NdotH, abs(NdotL), NdotV, bsdfData.roughness, partLambdaV)) * clampedNdotL;

    // Return the direct bsdf
    return diffR + specR;
}

#endif // DISNEY_HLSL