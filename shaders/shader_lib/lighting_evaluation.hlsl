#ifndef LIGHTING_EVALUATION_HLSL
#define LIGHTING_EVALUATION_HLSL

// Includes

float3 EvaluateDirectLighting(BSDFData bsdfData, float NdotV, float3 lightDir, float3 lightColor)
{
    return EvaluateBSDF(bsdfData, NdotV, lightDir) * lightColor;
}

#if !defined(SKY_NOT_SUPPORTED)

// Sky includes
#include "shader_lib/sky_utilities.hlsl"

float3 EvaluateIndirectLighting(BSDFData bsdfData, REAL3_DP positionPS, float3 lightDir, float planetRadius, Texture2D<float4> transmittanceLUTTexture, Texture2D<float4> multiScatteringLUTTexture, SamplerState samplerLinearClamp)
{
    // Indirect specular lighting
    float3 H = normalize(bsdfData.reflectedWS +  bsdfData.viewWS);
    float F_Ind = F_Schlick(0.02, dot(bsdfData.reflectedWS, H));
    
    // Change the unit to KMs
    positionPS = (positionPS / 1000.0);

    // We need to ensure that this is never lower than a certain radius.
    float3 luminance, throughput;
    IntegrateLuminanceThroughput(float3(positionPS), bsdfData.reflectedWS, lightDir, 0.0, 4, transmittanceLUTTexture, multiScatteringLUTTexture, samplerLinearClamp, luminance, throughput);
    return F_Ind * luminance * bsdfData.reflectionAttenuation * float3(0.9, 0.9, 0.95);
}
#endif

void AdjustReflectionVector(inout float3 R, float3 baseNormal, inout float attenuation)
{
    float RdotN = dot(R, baseNormal);
    if (RdotN < 0.0)
    {
        R = reflect(R, baseNormal);
        attenuation = 1.0;
    }
}

#endif // LIGHTING_EVALUATION_HLSL