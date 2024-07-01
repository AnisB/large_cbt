#ifndef SG_UTILITIES_H
#define SG_UTILITIES_H

#define NUM_SG_BANDS 4
#define SG_BAND_ATTENUATION_START 10.0f
#define SG_BAND_ATTENUATION_END 20.0f
#define BAND_ROUGHNESS_START 15.0f
#define BAND_ROUGHNESS_END 30.0f

// Water normal buffer
Texture2DArray<float4> _SurfaceGradientTexture: register(SURFACE_GRADIENT_BUFFER_BINDING_SLOT);

// Samplers
SamplerState surface_gradient_texture_sampler : register(SURFACE_GRADIENT_SAMPLER_BINDING_SLOT);

#if !defined(FP64_UNSUPPORTED)
float3 EvaluateNormal(double2 sampleUV, float3x3 localFrame, float distanceToCamera, float4 patchSize, uint patchFlags, bool attenuation = true)
{   
    float3 totalSG = float3(0.0, 0.0, 0.0);
    float activeBandCount = 0.0;
    float totalAtt = 0.0;

    // Accumulates the bands
    for (uint bandIdx = 0; bandIdx < NUM_SG_BANDS; ++bandIdx)
    {
        // Compute the UV coord
        float2 bandUV = float2(frac_double2(sampleUV / patchSize[bandIdx]));

        // Evaluate the derivatives for the sampling
        float2 uvDDX, uvDDY;
        evaluate_frac_derivatives(bandUV, uvDDX, uvDDY);

        // Read the texture
        float3 bandSG = _SurfaceGradientTexture.SampleGrad(surface_gradient_texture_sampler, float3(bandUV, bandIdx), uvDDX, uvDDY, 0).xyz;

        // Attenuation factor for this band
        float att = attenuation ? lerp(1.0, 0.0, saturate((distanceToCamera - patchSize[bandIdx] * SG_BAND_ATTENUATION_START) / (patchSize[bandIdx] * SG_BAND_ATTENUATION_END))) : 1.0;
        att *= (patchFlags >> bandIdx) & 0x1;

        // Accumulate the total and LF surface gradient
        totalSG += bandSG * att;
        totalAtt += att;

        // Count the number of active bands
        activeBandCount += att != 0.0 ? 1.0 : 0.0;
    }

    // Normalize the surface gradient
    if (activeBandCount > 1)
        totalSG /= max(totalAtt, 0.0000001);

    // Combine the surface gradients and convert to a normal
    float3 norm = normalize(float3(0, 1, 0) - totalSG);

    // Rotate to the local frame
    norm = mul(transpose(localFrame), norm);

    // Normalize and return
    return normalize(norm);
}
#else
float3 EvaluateNormal(float2 sampleUV, float3x3 localFrame, float distanceToCamera, float4 patchSize, uint patchFlags, bool attenuation = true)
{   
    float3 totalSG = float3(0.0, 0.0, 0.0);
    float activeBandCount = 0.0;
    float totalAtt = 0.0;

    // Accumulates the bands
    for (uint bandIdx = 0; bandIdx < NUM_SG_BANDS; ++bandIdx)
    {
        // Compute the UV coord
        float2 bandUV = float2(sampleUV / patchSize[bandIdx]);

        // Evaluate the derivatives for the sampling
        float2 uvDDX, uvDDY;
        evaluate_frac_derivatives(bandUV, uvDDX, uvDDY);

        // Read the texture
        float3 bandSG = _SurfaceGradientTexture.SampleGrad(surface_gradient_texture_sampler, float3(bandUV, bandIdx), uvDDX, uvDDY, 0).xyz;

        // Attenuation factor for this band
        float att = attenuation ? lerp(1.0, 0.0, saturate((distanceToCamera - patchSize[bandIdx] * SG_BAND_ATTENUATION_START) / (patchSize[bandIdx] * SG_BAND_ATTENUATION_END))) : 1.0;
        att *= (patchFlags >> bandIdx) & 0x1;

        // Accumulate the total and LF surface gradient
        totalSG += bandSG * att;
        totalAtt += att;

        // Count the number of active bands
        activeBandCount += att != 0.0 ? 1.0 : 0.0;
    }

    // Normalize the surface gradient
    if (activeBandCount > 1)
        totalSG /= max(totalAtt, 0.0000001);

    // Combine the surface gradients and convert to a normal
    float3 norm = normalize(float3(0, 1, 0) - totalSG);

    // Rotate to the local frame
    norm = mul(transpose(localFrame), norm);

    // Normalize and return
    return normalize(norm);
}
#endif

float EvaluateRoughness(float distanceToCamera, float4 patchSize, float4 patchRoughness)
{   
    // Initial roughness value
    float roughness = 0.000f;

    // Accumulates the bands
    for (uint bandIdx = 0; bandIdx < NUM_SG_BANDS; ++bandIdx)
    {
        // Attenuation factor for this band
        float roughnessFactor = lerp(0.0, 1.0, saturate((distanceToCamera - patchSize[bandIdx] * BAND_ROUGHNESS_START) / (patchSize[bandIdx] * BAND_ROUGHNESS_END)));

        // Approximate the roughness
        roughness += roughnessFactor * patchRoughness[bandIdx];
    }
    return roughness;
}

#endif