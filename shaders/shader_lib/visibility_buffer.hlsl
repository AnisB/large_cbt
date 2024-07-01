#ifndef VISIBILITY_BUFFER_HLSL
#define VISIBILITY_BUFFER_HLSL

uint4 pack_visibility_buffer(float2 barycentrics, uint materialID, uint triangleID)
{
    uint r = f32tof16(barycentrics.x);
    uint g = f32tof16(barycentrics.y);
    uint b = ((0xf & materialID) << 12) | (0xfff & (triangleID >> 16));
    uint a = (0xffff & triangleID);
    return uint4(r, g, b, a);
}

void unpack_visibility_buffer(uint4 visBuffer, out float3 barycentrics, out uint materialID, out uint triangleID)
{
    // Unpack the barycentrics
    barycentrics.x = f16tof32(visBuffer.x);
    barycentrics.y = f16tof32(visBuffer.y);
    barycentrics.z = 1.0 - barycentrics.x - barycentrics.y;

    // Unpack the triangle ID
    triangleID = ((0xfff & visBuffer.z) << 16) | visBuffer.w;

    // Unpack the material ID
    materialID = visBuffer.z >> 12;
}

#endif // VISIBILITY_BUFFER_HLSL