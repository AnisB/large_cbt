#ifndef OREN_NAYAR_HLSL
#define OREN_NAYAR_HLSL

// Includes
#include "shader_lib/bsdf.hlsl"

float3 EvaluateBSDF(BSDFData bsdfData, float NdotV, float3 lightDir)
{
    float NdotL = dot(bsdfData.normalWS, lightDir);

    float angleVN = acos(NdotV);
    float angleLN = acos(NdotL);

    float mu = 1.0; // roughness
    float A = 1.-.5*mu*mu/(mu*mu+0.57);
    float B = .45*mu*mu/(mu*mu+0.09);

    float alpha = max(angleVN, angleLN);
    float beta = min(angleVN, angleLN);
    float gamma = dot(bsdfData.viewWS -(bsdfData.normalWS * NdotV), lightDir - (bsdfData.normalWS * NdotL));
    float albedo = 1.1;
    float e0 = 3.1;
    float L1 = max(0.0, NdotL) * (A + B * max(0.0, gamma) * sin(alpha) * tan(beta));
    float3 col = L1 * bsdfData.diffuseColor;

    return col;
}

#endif // OREN_NAYAR_HLSL