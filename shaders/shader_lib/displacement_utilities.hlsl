#ifndef DISPLACEMENT_UTILITIES_H
#define DISPLACEMENT_UTILITIES_H

#define NUM_WATER_BANDS 4
#define DISPLACEMENT_BAND_ATTENUATION_START 10.0f
#define DISPLACEMENT_BAND_ATTENUATION_END 20.0f

// Water displacement buffers
Texture2DArray<float4> _DisplacementBuffer: register(DISPLACEMENT_TEXTURE_BINDING_SLOT);

// Samplers
SamplerState displacement_buffer_sampler : register(DISPLACEMENT_SAMPLER_BINDING_SLOT); 

#if !defined(FP64_UNSUPPORTED)
float3 EvaluateDisplacement(double2 sampleUV, float elevation, float distanceToCamera, float4 patchSize, float choppiness, uint patchFlags)
{   
    // Accumulation of the displacement
    float3 displacement = float3(0.0, 0.0, 0.0);

    // Evaluate the sampling UV
    double2 bandUV = frac_double2(sampleUV / double(patchSize.x));

    // For the first band, we do the bilinear interpolation manually due to interpolator float point precision issues
    double2 unnormalized = bandUV * 256;
    unnormalized.y -= 0.5;
    int2 tapCoord = (int2)floor_double2(floor_double2(unnormalized) + 0.5);

    // Read the 4 points (don't forget to wrap)
    float3 p0 = _DisplacementBuffer.Load(int4((tapCoord) & (256 - 1), 0, 0)).xyz;
    float3 p1 = _DisplacementBuffer.Load(int4((tapCoord + int2(1, 0)) & (256 - 1), 0, 0)).xyz;
    float3 p2 = _DisplacementBuffer.Load(int4((tapCoord + int2(0, 1)) & (256 - 1), 0, 0)).xyz;
    float3 p3 = _DisplacementBuffer.Load(int4((tapCoord + int2(1, 1)) & (256 - 1), 0, 0)).xyz;

    // Do the bilinear interpolation
    float2 fraction = float2(frac_double2(unnormalized));
    float3 i0 = lerp(p0, p1, fraction.x);
    float3 i1 = lerp(p2, p3, fraction.x);
    displacement = lerp(i0, i1, fraction.y);

    // Distance based attenuation
    displacement *= lerp(1.0, 0.0, saturate((distanceToCamera - patchSize.x * DISPLACEMENT_BAND_ATTENUATION_START) / (patchSize.x * DISPLACEMENT_BAND_ATTENUATION_END)));
    displacement *= (patchFlags.x & 0x1);

    // Process the other bands normall
    for (uint bandIdx = 1; bandIdx < NUM_WATER_BANDS; ++bandIdx)
    {
        // Evaluate the sampling UV
        double2 bandUV = frac_double2(sampleUV / double(patchSize[bandIdx]));

        // Read the displacement
        float3 bandDis = _DisplacementBuffer.SampleLevel(displacement_buffer_sampler, float3(bandUV, bandIdx), 0, 0).xyz;

        // Distance based attenuation
        float att = lerp(1.0, 0.0, saturate((distanceToCamera - patchSize[bandIdx] * DISPLACEMENT_BAND_ATTENUATION_START) / (patchSize[bandIdx] * DISPLACEMENT_BAND_ATTENUATION_END)));
        att *= (patchFlags >> bandIdx) & 0x1;

        // Shuffle into the right order and add
        displacement += bandDis * att;
    }

    // Swizzle the deformations
    displacement = float3(-displacement.y, displacement.x, -displacement.z);

    // Apply the choppiness
    displacement.xz *= lerp(0.0, choppiness, elevation);

    // Apply the local transformation
    return displacement;
}
#else
float3 EvaluateDisplacement(float2 sampleUV, float elevation, float distanceToCamera, float4 patchSize, float choppiness, float4 patchFlag)
{   
    // Accumulation of the displacement
    float3 displacement = float3(0.0, 0.0, 0.0);
    
    // Process the other bands normall
    for (uint bandIdx = 0; bandIdx < NUM_WATER_BANDS; ++bandIdx)
    {
        // Evaluate the sampling UV
        float2 bandUV = sampleUV / patchSize[bandIdx];

        // Read the displacement
        float3 bandDis = _DisplacementBuffer.SampleLevel(displacement_buffer_sampler, float3(bandUV, bandIdx), 0, 0).xyz;

        // Distance based attenuation
        float att = lerp(1.0, 0.0, saturate((distanceToCamera - patchSize[bandIdx] * DISPLACEMENT_BAND_ATTENUATION_START) / (patchSize[bandIdx] * DISPLACEMENT_BAND_ATTENUATION_END)));
        att *= patchFlag[bandIdx];

        // Shuffle into the right order and add
        displacement += bandDis * att;
    }

    // Swizzle the deformations
    displacement = float3(-displacement.y, displacement.x, -displacement.z);

    // Apply the choppiness
    displacement.xz *= lerp(0.0, choppiness, elevation);

    // Apply the local transformation
    return displacement;
}
#endif

#endif // DISPLACEMENT_UTILITIES_H
