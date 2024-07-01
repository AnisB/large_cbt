#ifndef MOON_DETAIL_HLSL
#define MOON_DETAIL_HLSL

// Attenuation range for the detail elevation
#define SG_BAND_ATTENUATION_START 15.0f
#define SG_BAND_ATTENUATION_END 30.0f

// Size between the group of bands
#define BAND_GROUP_SIZE_FACTOR 16.0

// Water displacement buffers
Texture2D<float> _DetailBuffer: register(DETAIL_TEXTURE_BINDING_SLOT);

// Sampler
SamplerState detail_buffer_sampler : register(DETAIL_SAMPLER_BINDING_SLOT);

// Function that 
void combine_band_displacement(float3 positionRWS, float3 normalWS, float detail, uint attenuation, inout float waveLength, inout float amplitude, inout float displacement)
{
    // Compute the position with all the previous bands
    float3 currentPositionRWS = positionRWS + detail * normalWS;

    // Distance to camera
    float distanceToCamera = length(currentPositionRWS);

    // Attenuation for this band
    float att = attenuation ? lerp(1.0, 0.0, saturate((distanceToCamera - waveLength * SG_BAND_ATTENUATION_START) / (waveLength * SG_BAND_ATTENUATION_END))) : 1.0;

    // Shuffle into the right order and add
    displacement += detail * att;

    // Prepare for the next band
    waveLength = waveLength / 2;
    amplitude = amplitude / 2;
}

void EvaluateDetail(REAL2_DP sampleUV, float patchSize, float patchAmplitude, uint numOctaves, float3 positionRWS, float3 normalWS, uint attenuation, inout float displacement)
{   
    // Initialize the accumulation values
    float waveLength = patchSize;
    float amplitude = patchAmplitude;
    uint bandIdx = 0;

    // Process the first group of bands
    for (; bandIdx < min(2, numOctaves); ++bandIdx)
    {
        #if !defined(FP64_UNSUPPORTED)
        double2 bandUV = frac_double2(sampleUV / double(waveLength));
        #else
        float2 bandUV = frac(sampleUV / waveLength);
        #endif

        // Read the displacement (in double for the artifacts)
        float bandDetail = bilinear_interpolation(_DetailBuffer, bandUV) * amplitude;
        combine_band_displacement(positionRWS, normalWS, bandDetail, attenuation,waveLength, amplitude, displacement);
    }

    // Prepare for the next group of bands
    waveLength = waveLength / BAND_GROUP_SIZE_FACTOR;
    amplitude = amplitude / BAND_GROUP_SIZE_FACTOR;

    // Process the second group of bands
    for (; bandIdx < numOctaves; ++bandIdx)
    {
        // Evaluate the sampling UV
        #if !defined(FP64_UNSUPPORTED)
        double2 bandUV = frac_double2(sampleUV / double(waveLength));
        #else
        float2 bandUV = frac(sampleUV / waveLength);
        #endif

        // Read the displacement
        float bandDetail = bilinear_interpolation(_DetailBuffer, bandUV) * amplitude;
        combine_band_displacement(positionRWS, normalWS, bandDetail, attenuation,waveLength, amplitude, displacement);
    }
}

#endif // MOON_DETAIL_HLSL
