#ifndef COLOR_HLSL
#define COLOR_HLSL

float3 linear_to_srgb(float3 c)
{
    float3 sRGBLo = c * 12.92;
    float3 sRGBHi = (pow(c, float3(1.0/2.4, 1.0/2.4, 1.0/2.4)) * 1.055) - 0.055;
    float3 sRGB   = select(c <= 0.0031308, sRGBLo, sRGBHi);
    return sRGB;
}

float3 srgb_to_linear(float3 c)
{
    float3 linearRGBLo  = c / 12.92;
    float3 linearRGBHi  = pow((c + 0.055) / 1.055, float3(2.4, 2.4, 2.4));
    float3 linearRGB    = select(c <= 0.04045, linearRGBLo, linearRGBHi);
    return linearRGB;
}

float luminance(float3 color)
{
    return dot(color, float3(0.2126729, 0.7151522, 0.0721750));
}

#endif // COLOR_HLSL