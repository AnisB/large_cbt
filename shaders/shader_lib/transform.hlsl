#ifndef TRANSFORM_HLSL
#define TRANSFORM_HLSL

// NDC to clip Space
float4 evaluate_clip_space_position(float2 positionNDC, float depthValue)
{
    float4 positionCS = float4(positionNDC * 2.0 - 1.0, depthValue, 1.0);
    positionCS.y = -positionCS.y;
    return positionCS;
}

// NDC to RWS
float3 evaluate_world_space_position(float2 positionNDC, float depthValue, float4x4 invViewProjMatrix)
{
    float4 positionCS  = evaluate_clip_space_position(positionNDC, depthValue);
    float4 hpositionWS = mul(invViewProjMatrix, positionCS);
    return hpositionWS.xyz / hpositionWS.w;
}

float3 get_ray_direction(float2 positionNDC, float4x4 invViewProjMatrix)
{
    float3 virtualPosition = evaluate_world_space_position(positionNDC, 0.5, invViewProjMatrix);
    float distanceToCamera = length(virtualPosition);
    return virtualPosition / max(distanceToCamera, 0.00001);
}

// Normalized planet space to long lat
float2 normalized_coordinates_to_longlat(float3 positionNPS)
{
    float lat = asin(positionNPS.z) / PI + 0.5;
    float lon = atan2(positionNPS.y, positionNPS.x) / (2.0 * PI) + 0.5;
    return float2(lon, lat);
}

#if !defined(FP64_UNSUPPORTED)
double2 normalized_coordinates_to_longlat(double3 positionNPS)
{
    double lat = asin_double(positionNPS.z) / PI + 0.5;
    double lon = atan2_double(positionNPS.y, positionNPS.x) / (2.0 * PI) + 0.5;
    return double2(lon, lat);
}
#endif

// Long/Lat to normalized planet space
float3 longlat_to_normalized_coordinates(float u, float v)
{
    float lon = (u - 0.5) * (2.0 * PI);
    float lat = (v - 0.5) * PI;
    float cosLat = cos(lat);
    float sinLat = sin(lat);
    float sinLon = sin(lon);
    float cosLon = cos(lon);
    float x = cosLon * cosLat;
    float y = sinLon * cosLat;
    float z = sinLat;
    return float3( x, y, z );
}

// Orientation to direction
float2 orientation_to_direction(float orientation)
{
    return float2(cos(orientation), sin(orientation));
}

// Clip space to pixel
float2 clip_space_to_pixel(float4 positionCS, float2 screenSize)
{
    float2 p = positionCS.xy / positionCS.w;
    p.y = -p.y;
    p.xy = (p.xy * 0.5 + 0.5);
    p.xy *= screenSize.xy;
    return p.xy;
}

// Planet space to normalized planet space
float3 evaluate_normalized_planet_space(float3 positionPS, float planetRadius)
{
    float3 posNPS = positionPS / planetRadius;
    return posNPS.y == 1.0 ? float3(0, 1, 0) : posNPS / length(posNPS);
}

#if !defined(FP64_UNSUPPORTED)
double3 evaluate_normalized_planet_space(double3 positionPS, double planetRadius)
{
    double3 posNPS = positionPS / planetRadius;
    return posNPS.y == 1.0 ? double3(0, 1, 0) : posNPS * invsqrt_double(dot_double(posNPS, posNPS));
}
#endif

float2 project_position_to_disk(float3 posNPS)
{
    float v = abs(posNPS.y) + 1e-10;
    // The acos is not viable close to the origin, we can actually use the coords straight away when we are close to the origin.
    float r = acos(v) / HALF_PI;
    float s = (posNPS.x * posNPS.x + posNPS.z * posNPS.z);
    float p = s != 0.0 ? 1.0 / sqrt(posNPS.x * posNPS.x + posNPS.z * posNPS.z) * r :  0.0;
    float up = posNPS.x * p;
    float vp = posNPS.z * p;
    return float2(up, vp);
}

#if !defined(FP64_UNSUPPORTED)
double2 project_position_to_disk(double3 posNPS)
{
    double v = abs(posNPS.y) + 1e-10;
    // The acos is not viable close to the origin, we can actually use the coords straight away when we are close to the origin.
    if (v <= 0.999)
    {
        // Normalize the coordinates
        double r = acos_double(v) / HALF_PI;
        double s = (posNPS.x * posNPS.x + posNPS.z * posNPS.z);
        double p = s != 0.0 ? invsqrt_double(posNPS.x * posNPS.x + posNPS.z * posNPS.z) * r :  0.0;
        double u = posNPS.x * p;
        double v = posNPS.z * p;
        return double2(u, v);
    }
    return posNPS.xz / HALF_PI;
}
#endif

float3 project_disk_to_position(float2 uv)
{
    float r = sqrt(uv.x * uv.x + uv.y * uv.y);
    float theta = r * HALF_PI;
    float2 sinCosTheta;
    sinCosTheta.x = sin(theta);
    sinCosTheta.y = cos(theta);
    float x = uv.x / r * sinCosTheta.x;
    float y = sinCosTheta.y;
    float z = uv.y / r * sinCosTheta.x;
    return float3(x,y,z);
}

#if !defined(FP64_UNSUPPORTED)
double3 project_disk_to_position(double2 uv)
{
    double r = sqrt_double(uv.x * uv.x + uv.y * uv.y);
    double theta = r * HALF_PI;
    double2 sinCosTheta = sincos_double(theta);
    double x = uv.x / r * sinCosTheta.x;
    double y = sinCosTheta.y;
    double z = uv.y / r * sinCosTheta.x;
    return double3(x,y,z);
}
#endif

float3x3 get_local_frame(float3 posNPS, float2 uv)
{   
    // In case we are close to the origin, we don't need to evaluate the local frame
    float u2 = uv.x * uv.x;
    float v2 = uv.y * uv.y;
    float u2v2 = u2 + v2;
    float sqrt_u2v2 = sqrt(u2 + v2);
    float u2v2_32 = (sqrt_u2v2 * u2v2);
    float T = (HALF_PI * sqrt_u2v2);
    float B = T < 1e-5 ? T : sin(T);
    float A = T < 1e-5 ? 1.0 - T : cos(T);

    // Tangent
    float tu_x = PI * u2 * A / (2 * u2v2) + v2 * B / u2v2_32;
    float tu_y = -PI * uv.x * B / (2.0 * sqrt_u2v2);
    float tu_z = 0.5 * uv.x * uv.y * (PI * A / u2v2 - 2.0 * B / u2v2_32);

    // Bitangent
    float btv_x = 0.5 * uv.x * uv.y * (PI * A / u2v2 - 2.0 * B / u2v2_32);
    float btv_y = -PI * uv.y * B / (2.0 * sqrt_u2v2);
    float btv_z = PI * v2 * A / (2 * u2v2) + u2 * B / u2v2_32;

    // Normalize the results
    float3 tang, bitang;
    if (abs(uv.x) >= 1e-7)
        tang = normalize(float3(tu_x, tu_y, tu_z));
    else
        tang = float3(1, 0, 0);

    if (abs(uv.y) >= 1e-7)
        bitang = normalize(float3(btv_x, btv_y, btv_z));
    else
        bitang = float3(0, 0, 1);

    // Flip operation in case we are in the lower hemisphere
    if (posNPS.y < 0.0)
    {
        tang.y = -tang.y;
        bitang.y = -bitang.y;
    }

    // return the basis
    return float3x3(tang, posNPS, bitang);
}

float3 evaluate_surface_gradients(float3 p0, float3 p1, float3 p2)
{
    float3 v0 = (p1 - p0);
    float3 v1 = (p2 - p0);
    return surface_gradient_from_perturbed_normal(float3(0, 1, 0), normalize(cross(v1, v0)));
}

#endif // TRANSFORM_HLSL