#ifndef FRUSTUM_CULLING_H
#define FRUSTUM_CULLING_H

struct Plane
{
   float3 normal;
   float d;
};

bool FrustumAABBIntersect(in Plane frustumPlanes[6], float3 aabbMin, float3 aabbMax)
{
   float3 center = (aabbMax + aabbMin) * 0.5;
   float3 extents = (aabbMax - aabbMin) * 0.5;
    for (int i = 0; i < 4; i++)
    {
        Plane plane = frustumPlanes[i];
        float3 normal_sign = sign(plane.normal);
        float3 test_point = center + extents * normal_sign;
 
        float dotProd = dot(test_point, plane.normal);
        if (dotProd + plane.d < 0)
            return false;
    }
    return true;
}

#endif //FRUSTUM_CULLING_H
