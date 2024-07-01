#ifndef SAMPLING_HLSL
#define SAMPLING_HLSL

#if !defined(FP64_UNSUPPORTED)
float bilinear_interpolation(Texture2D<float> targetTexture, double2 uv)
{
    // Get the texture size
    uint width, height;
    targetTexture.GetDimensions(width, height);

    // For the first band, we do the bilinear interpolation manually due to interpolator float point precision issues
    double2 unnormalized = uv * float2(width, height);
    unnormalized.xy -= 0.5;
    int2 tapCoord = (int2)floor_double2(floor_double2(unnormalized) + 0.5);

    // Read the 4 points (don't forget to wrap)
    float p0 = targetTexture.Load(int3(repeat_coord(tapCoord, width, height), 0));
    float p1 = targetTexture.Load(int3(repeat_coord(tapCoord + int2(1, 0), width, height), 0));
    float p2 = targetTexture.Load(int3(repeat_coord(tapCoord + int2(0, 1), width, height), 0));
    float p3 = targetTexture.Load(int3(repeat_coord(tapCoord + int2(1, 1), width, height), 0));

    // Do the bilinear interpolation
    float2 fraction = float2(frac_double2(unnormalized));
    float i0 = lerp(p0, p1, fraction.x);
    float i1 = lerp(p2, p3, fraction.x);
    return lerp(i0, i1, fraction.y);
}

float4 bilinear_interpolation(Texture2D<float4> targetTexture, double2 uv)
{
    // Get the texture size
    uint width, height;
    targetTexture.GetDimensions(width, height);

    // For the first band, we do the bilinear interpolation manually due to interpolator float point precision issues
    double2 unnormalized = uv * float2(width, height);
    unnormalized.xy -= 0.5;
    int2 tapCoord = (int2)floor_double2(floor_double2(unnormalized) + 0.5);

    // Read the 4 points (don't forget to wrap)
    float4 p0 = targetTexture.Load(int3(repeat_coord(tapCoord, width, height), 0));
    float4 p1 = targetTexture.Load(int3(repeat_coord(tapCoord + int2(1, 0), width, height), 0));
    float4 p2 = targetTexture.Load(int3(repeat_coord(tapCoord + int2(0, 1), width, height), 0));
    float4 p3 = targetTexture.Load(int3(repeat_coord(tapCoord + int2(1, 1), width, height), 0));

    // Do the bilinear interpolation
    float2 fraction = float2(frac_double2(unnormalized));
    float4 i0 = lerp(p0, p1, fraction.x);
    float4 i1 = lerp(p2, p3, fraction.x);
    return lerp(i0, i1, fraction.y);
}
#else
float bilinear_interpolation(Texture2D<float> targetTexture, float2 uv)
{
    // Get the texture size
    uint width, height;
    targetTexture.GetDimensions(width, height);

    // For the first band, we do the bilinear interpolation manually due to interpolator float point precision issues
    float2 unnormalized = uv * float2(width, height);
    unnormalized.xy -= 0.5;
    int2 tapCoord = (int2)floor(floor(unnormalized) + 0.5);

    // Read the 4 points (don't forget to wrap)
    float p0 = targetTexture.Load(int3(repeat_coord(tapCoord, width, height), 0));
    float p1 = targetTexture.Load(int3(repeat_coord(tapCoord + int2(1, 0), width, height), 0));
    float p2 = targetTexture.Load(int3(repeat_coord(tapCoord + int2(0, 1), width, height), 0));
    float p3 = targetTexture.Load(int3(repeat_coord(tapCoord + int2(1, 1), width, height), 0));

    // Do the bilinear interpolation
    float2 fraction = float2(frac(unnormalized));
    float i0 = lerp(p0, p1, fraction.x);
    float i1 = lerp(p2, p3, fraction.x);
    return lerp(i0, i1, fraction.y);
}

float4 bilinear_interpolation(Texture2D<float4> targetTexture, float2 uv)
{
    // Get the texture size
    uint width, height;
    targetTexture.GetDimensions(width, height);

    // For the first band, we do the bilinear interpolation manually due to interpolator float point precision issues
    float2 unnormalized = uv * float2(width, height);
    unnormalized.xy -= 0.5;
    int2 tapCoord = (int2)floor(floor(unnormalized) + 0.5);

    // Read the 4 points (don't forget to wrap)
    float4 p0 = targetTexture.Load(int3(repeat_coord(tapCoord, width, height), 0));
    float4 p1 = targetTexture.Load(int3(repeat_coord(tapCoord + int2(1, 0), width, height), 0));
    float4 p2 = targetTexture.Load(int3(repeat_coord(tapCoord + int2(0, 1), width, height), 0));
    float4 p3 = targetTexture.Load(int3(repeat_coord(tapCoord + int2(1, 1), width, height), 0));

    // Do the bilinear interpolation
    float2 fraction = float2(frac(unnormalized));
    float4 i0 = lerp(p0, p1, fraction.x);
    float4 i1 = lerp(p2, p3, fraction.x);
    return lerp(i0, i1, fraction.y);
}

#endif
#endif // SAMPLING_HLSL
