#ifndef MOON_DETAIL_NORMAL_HLSL
#define MOON_DETAIL_NORMAL_HLSL

// Attenuation range for the detail elevation
#define SLOPE_BAND_ATTENUATION_START 20.0f 
#define SLOPE_BAND_ATTENUATION_END 25.0f

// Size between the group of bands
#define BAND_GROUP_SIZE_FACTOR 16.0

// Arbitrary attenuation
#define SLOPE_COMPENSATION_FACTOR 1.0

// Water displacement buffers
Texture2D<float2> _DetailSlopeBuffer: register(DETAIL_SG_TEXTURE_BINDING_SLOT);

// Sampler
SamplerState detail_sg_buffer_sampler : register(DETAIL_SG_SAMPLER_BINDING_SLOT);

// Function that 
void combine_band_slopes(float distanceToCamera, float2 slope, uint attenuation, inout float waveLength, inout float amplitude, inout float2 totalSlope)
{
    // Rescale
    slope = slope * amplitude / waveLength * 0.5;

    // Evaluate the band's attenuation
    float att = attenuation ? lerp(1.0, 0.0, saturate((distanceToCamera - waveLength * SLOPE_BAND_ATTENUATION_START) / (waveLength * SLOPE_BAND_ATTENUATION_END))) : 1.0;

    // Shuffle into the right order and add
    totalSlope += slope * att * SLOPE_COMPENSATION_FACTOR;

    // Prepare for the next band
    waveLength = waveLength / 2;
    amplitude = amplitude / 2;
}

float3 EvaluateDetailSG(REAL2_DP sampleUV, float patchSize, float patchAmplitude, uint numOctaves, float3x3 localFrame, float distanceToCamera, bool attenuation)
{   
    // Accumulation of the detail
    float2 totalSlope = 0.0;

    // Init the values for the bands
    float waveLength = patchSize;
    float amplitude = patchAmplitude;
    uint bandIdx = 0;

    // Process the other bands normall
    for (; bandIdx < min(2, numOctaves); ++bandIdx)
    {
        #if !defined(FP64_UNSUPPORTED)
        double2 bandUV = frac_double2(sampleUV / double(waveLength));
        #else
        float2 bandUV = frac(sampleUV / waveLength);
        #endif

        // Read the slopes
        float2 slope = _DetailSlopeBuffer.Sample(detail_sg_buffer_sampler, float2(bandUV), 0);
        combine_band_slopes(distanceToCamera, slope, attenuation, waveLength, amplitude, totalSlope);
    }

    // Prepare for the next group of bands
    waveLength = waveLength / BAND_GROUP_SIZE_FACTOR;
    amplitude = amplitude / BAND_GROUP_SIZE_FACTOR;

    // Process the other bands normall
    for (; bandIdx < numOctaves; ++bandIdx)
    {
        #if !defined(FP64_UNSUPPORTED)
        double2 bandUV = frac_double2(sampleUV / double(waveLength));
        #else
        float2 bandUV = frac(sampleUV / waveLength);
        #endif

        // Read the slopes
        float2 slope = _DetailSlopeBuffer.Sample(detail_sg_buffer_sampler, float2(bandUV), 0);
        combine_band_slopes(distanceToCamera, slope, attenuation, waveLength, amplitude, totalSlope);
    }

    // Combine the surface gradients and convert to a normal
    float3 norm = normalize(float3(0, 1, 0) + float3(-totalSlope.x, 0, totalSlope.y));

    // Rotate to the local frame
    norm = mul(transpose(localFrame), norm);

    // Normalize and return
    return norm;
}

#endif // MOON_DETAIL_NORMAL_HLSL
