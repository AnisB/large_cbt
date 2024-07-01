#ifndef BSDF_H
#define BSDF_H

struct BSDFData
{
    float3 diffuseColor;
    float roughness;
    float3 normalWS;
    float3 reflectedWS;
    float reflectionAttenuation;
    float3 viewWS;
};

struct LightingData
{
    float3 lightDir;
    float3 viewWS;
    float NdotL;
};

#endif // BSDF_H