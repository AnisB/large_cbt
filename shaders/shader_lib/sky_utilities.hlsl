#ifndef SKY_UTILITIES_H
#define SKY_UTILITIES_H

// Global defines
#define TRANSMITTANCE_TEXTURE_WIDTH 256
#define TRANSMITTANCE_TEXTURE_HEIGHT 64
#define MULTI_SCATTERING_TEXTURE_RESOLUTION 32
#define PLANET_RADIUS_OFFSET 0.01f
#define SAMPLE_COUNT_INI_OPTIAL_DEPTH 40.0
#define SAMPLE_COUNT_INI_MULTI_SCAT 20.0
#define SAMPLE_COUNT_INI_SKY_VIEW 30.0
#define MULTIPLE_SCATTERING_FACTOR 1.0f
#define SKY_INTENSITY_MUTLIPLIER 3.0
#define SKY_VIEW_LUT_TEXTURE_WIDTH 192
#define SKY_VIEW_LUT_TEXTURE_HEIGHT 108

cbuffer SkyAtmosphereCB : register(SKY_ATMOSPHERE_BUFFER_SLOT)
{
    float3  _AbsorptionExtinction;
    float _PlanetRadiusSky;

    float3  _RayleighScattering;
    float   _MisePhaseFunctionG;

    float3  _MieScattering;
    float   _BottomRadius;

    float3  _MieExtinction;
    float   _TopRadius;

    float3  _MieAbsorption;
    float   _MiePhaseG;
    
    float3 _GroundAlbedo;
    float _PaddingSA0;

    float3 _PlanetCenterSky;
    float _PaddingSA1;

    float _RayleighDensity0LayerWidth;
    float _RayleighDensity0ExpTerm;
    float _RayleighDensity0ExpScale;
    float _RayleighDensity0LinearTerm;
    float _RayleighDensity0ConstantTerm;

    float _RayleighDensity1LayerWidth;
    float _RayleighDensity1ExpTerm;
    float _RayleighDensity1ExpScale;
    float _RayleighDensity1LinearTerm;
    float _RayleighDensity1ConstantTerm;

    float _MieDensity0LayerWidth;
    float _MieDensity0ExpTerm;
    float _MieDensity0ExpScale;
    float _MieDensity0LinearTerm;
    float _MieDensity0ConstantTerm;

    float _MieDensity1LayerWidth;
    float _MieDensity1ExpTerm;
    float _MieDensity1ExpScale;
    float _MieDensity1LinearTerm;
    float _MieDensity1ConstantTerm;

    float _AbsorptionDensity0LayerWidth;
    float _AbsorptionDensity0ExpTerm;
    float _AbsorptionDensity0ExpScale;
    float _AbsorptionDensity0LinearTerm;
    float _AbsorptionDensity0ConstantTerm;

    float _AbsorptionDensity1LayerWidth;
    float _AbsorptionDensity1ExpTerm;
    float _AbsorptionDensity1ExpScale;
    float _AbsorptionDensity1LinearTerm;
    float _AbsorptionDensity1ConstantTerm;
};

float CornetteShanksMiePhaseFunction(float g, float cosTheta)
{
    float k = 3.0 / (8.0 * PI) * (1.0 - g * g) / (2.0 + g * g);
    return k * (1.0 + cosTheta * cosTheta) / pow(1.0 + g * g - 2.0 * g * -cosTheta, 1.5);
}

float RayleighPhase(float cosTheta)
{
    float factor = 3.0f / (16.0f * PI);
    return factor * (1.0f + cosTheta * cosTheta);
}

float GetAlbedo(float scattering, float extinction)
{
    return scattering / max(0.001, extinction);
}

float3 GetAlbedo(float3 scattering, float3 extinction)
{
    return scattering / max(0.001, extinction);
}

struct MediumSampleRGB
{
    float3 scattering;
    float3 absorption;
    float3 extinction;

    float3 scatteringMie;
    float3 absorptionMie;
    float3 extinctionMie;

    float3 scatteringRay;
    float3 absorptionRay;
    float3 extinctionRay;

    float3 scatteringOzo;
    float3 absorptionOzo;
    float3 extinctionOzo;

    float3 albedo;
};

MediumSampleRGB SampleMediumRGB(in float3 WorldPos)
{
    const float viewHeight = length(WorldPos) - _BottomRadius;

    const float densityMie = exp(_MieDensity1ExpScale * viewHeight);
    const float densityRay = exp(_RayleighDensity1ExpScale * viewHeight);
    const float densityOzo = saturate(viewHeight < _AbsorptionDensity0LayerWidth ?
        _AbsorptionDensity0LinearTerm * viewHeight + _AbsorptionDensity0ConstantTerm :
        _AbsorptionDensity1LinearTerm * viewHeight + _AbsorptionDensity1ConstantTerm);

    MediumSampleRGB s;

    s.scatteringMie = densityMie * _MieScattering;
    s.absorptionMie = densityMie * _MieAbsorption;
    s.extinctionMie = densityMie * _MieExtinction;

    s.scatteringRay = densityRay * _RayleighScattering;
    s.absorptionRay = 0.0f;
    s.extinctionRay = s.scatteringRay + s.absorptionRay;

    s.scatteringOzo = 0.0;
    s.absorptionOzo = densityOzo * _AbsorptionExtinction;
    s.extinctionOzo = s.scatteringOzo + s.absorptionOzo;

    s.scattering = s.scatteringMie + s.scatteringRay + s.scatteringOzo;
    s.absorption = s.absorptionMie + s.absorptionRay + s.absorptionOzo;
    s.extinction = s.extinctionMie + s.extinctionRay + s.extinctionOzo;
    s.albedo = GetAlbedo(s.scattering, s.extinction);

    return s;
}

float fromUnitToSubUvs(float u, float resolution)
{
    return (u + 0.5f / resolution) * (resolution / (resolution + 1.0f));
}

float fromSubUvsToUnit(float u, float resolution)
{
    return (u - 0.5f / resolution) * (resolution / (resolution - 1.0f));
}

void UvToLutTransmittanceParams(in float2 uv, out float viewHeight, out float viewZenithCosAngle)
{
    float x_mu = uv.x;
    float x_r = uv.y;

    float H = sqrt(_TopRadius * _TopRadius - _BottomRadius * _BottomRadius);
    float rho = H * x_r;
    viewHeight = sqrt(rho * rho + _BottomRadius * _BottomRadius);

    float d_min = _TopRadius - viewHeight;
    float d_max = rho + H;
    float d = d_min + x_mu * (d_max - d_min);
    viewZenithCosAngle = d == 0.0 ? 1.0f : (H * H - rho * rho - d * d) / (2.0 * viewHeight * d);
    viewZenithCosAngle = clamp(viewZenithCosAngle, -1.0, 1.0);
}

void LutTransmittanceParamsToUv(in float viewHeight, in float viewZenithCosAngle, out float2 uv)
{
    float H = sqrt(max(0.0f, _TopRadius * _TopRadius - _BottomRadius * _BottomRadius));
    float rho = sqrt(max(0.0f, viewHeight * viewHeight - _BottomRadius * _BottomRadius));

    float discriminant = viewHeight * viewHeight * (viewZenithCosAngle * viewZenithCosAngle - 1.0) + _TopRadius * _TopRadius;
    float d = max(0.0, (-viewHeight * viewZenithCosAngle + sqrt(discriminant))); // Distance to atmosphere boundary

    float d_min = _TopRadius - viewHeight;
    float d_max = rho + H;
    float x_mu = (d - d_min) / (d_max - d_min);
    float x_r = rho / H;

    uv = float2(x_mu, x_r);
}

void UvToSkyViewLutParams(out float viewZenithCosAngle, out float lightViewCosAngle, in float viewHeight, in float2 uv)
{
    // Constrain uvs to valid sub texel range (avoid zenith derivative issue making LUT usage visible)
    uv = float2(fromSubUvsToUnit(uv.x, SKY_VIEW_LUT_TEXTURE_WIDTH), fromSubUvsToUnit(uv.y, SKY_VIEW_LUT_TEXTURE_HEIGHT));

    float Vhorizon = sqrt(viewHeight * viewHeight - _BottomRadius * _BottomRadius);
    float CosBeta = Vhorizon / viewHeight;              // GroundToHorizonCos
    float Beta = acos(CosBeta);
    float ZenithHorizonAngle = PI - Beta;

    if (uv.y < 0.5f)
    {
        float coord = 2.0*uv.y;
        coord = 1.0 - coord;
        coord *= coord;
        coord = 1.0 - coord;
        viewZenithCosAngle = cos(ZenithHorizonAngle * coord);
    }
    else
    {
        float coord = uv.y*2.0 - 1.0;
        coord *= coord;
        viewZenithCosAngle = cos(ZenithHorizonAngle + Beta * coord);
    }

    float coord = uv.x;
    coord *= coord;
    lightViewCosAngle = -(coord*2.0 - 1.0);
}

void SkyViewLutParamsToUv(in bool IntersectGround, in float viewZenithCosAngle, in float lightViewCosAngle, in float viewHeight, out float2 uv)
{
    float Vhorizon = sqrt(viewHeight * viewHeight - _BottomRadius * _BottomRadius);
    float CosBeta = Vhorizon / viewHeight;              // GroundToHorizonCos
    float Beta = acos(CosBeta);
    float ZenithHorizonAngle = PI - Beta;

    if (!IntersectGround)
    {
        float coord = acos(viewZenithCosAngle) / ZenithHorizonAngle;
        coord = 1.0 - coord;
        coord = sqrt(max(coord, 0));
        coord = 1.0 - coord;
        uv.y = coord * 0.5f;
    }
    else
    {
        float coord = (acos(viewZenithCosAngle) - ZenithHorizonAngle) / Beta;
        coord = sqrt(coord);
        uv.y = coord * 0.5f + 0.5f;
    }

    {
        float coord = -lightViewCosAngle * 0.5f + 0.5f;
        coord = sqrt(coord);
        uv.x = coord;
    }

    // Constrain uvs to valid sub texel range (avoid zenith derivative issue making LUT usage visible)
    uv = float2(fromUnitToSubUvs(uv.x, SKY_VIEW_LUT_TEXTURE_WIDTH), fromUnitToSubUvs(uv.y, SKY_VIEW_LUT_TEXTURE_HEIGHT));
}

float3 IntegrateOpticalDepth(in float2 pixPos, in float3 WorldPos, in float3 WorldDir)
{
    // Compute next intersection with atmosphere or ground 
    float3 earthO = _PlanetCenterSky;
    float tBottom = ray_sphere_intersect_nearest(WorldPos, WorldDir, earthO, _BottomRadius);
    float tTop = ray_sphere_intersect_nearest(WorldPos, WorldDir, earthO, _TopRadius);
    float tMax = 0.0f;
    if (tBottom < 0.0f)
    {
        if (tTop < 0.0f)
            return 0.0;
        else
            tMax = tTop;
    }
    else if (tTop > 0.0f)
        tMax = min(tTop, tBottom);
    tMax = min(tMax, 9000000.0f);

    // Step size
    float dt = tMax / SAMPLE_COUNT_INI_OPTIAL_DEPTH;

    // Ray march the atmosphere to integrate optical depth
    float3 OpticalDepth = 0.0;
    float t = 0.0f;
    float tPrev = 0.0;
    const float SampleSegmentT = 0.3f;
    for (float s = 0.0f; s < SAMPLE_COUNT_INI_OPTIAL_DEPTH; s += 1.0f)
    {
        // Exact difference, important for accuracy of multiple scattering
        float NewT = tMax * (s + SampleSegmentT) / SAMPLE_COUNT_INI_OPTIAL_DEPTH;
        dt = NewT - t;
        t = NewT;
        float3 P = WorldPos + t * WorldDir;

        // Compute the otpical depth
        MediumSampleRGB medium = SampleMediumRGB(P);
        const float3 SampleOpticalDepth = medium.extinction * dt;
        const float3 SampleTransmittance = exp(-SampleOpticalDepth);
        OpticalDepth += SampleOpticalDepth;

        // Next step
        tPrev = t;
    }
    return OpticalDepth;
}

void IntegrateLuminanceMultiScatt(float2 pixPos, in float3 WorldPos, in float3 WorldDir, in float3 SunDir,
                                    Texture2D<float4> transmittanceLUTTexture, SamplerState samplerLinearClamp,
                                    out float3 luminance, out float3 multiScat)
{
    // Initialize the outputs
    luminance = float3(0.0, 0.0, 0.0);
    multiScat = float3(0.0, 0.0, 0.0);

    // Compute next intersection with atmosphere or ground 
    float3 earthO = _PlanetCenterSky;
    float tBottom = ray_sphere_intersect_nearest(WorldPos, WorldDir, earthO, _BottomRadius);
    float tTop = ray_sphere_intersect_nearest(WorldPos, WorldDir, earthO, _TopRadius);
    float tMax = 0.0f;
    if (tBottom < 0.0f)
    {
        if (tTop < 0.0f)
            return;
        else
            tMax = tTop;
    }
    else if (tTop > 0.0f)
        tMax = min(tTop, tBottom);
    tMax = min(tMax, 9000000.0f);

    // Step size
    float dt = tMax / SAMPLE_COUNT_INI_MULTI_SCAT;

    // Phase functions
    const float uniformPhase = 1.0 / (4.0 * PI);
    const float3 wi = SunDir;
    const float3 wo = WorldDir;
    float cosTheta = dot(wi, wo);

    // Ray march the atmosphere to integrate optical depth
    float3 L = 0.0f;
    float3 throughput = 1.0;
    float t = 0.0f;
    float tPrev = 0.0;
    const float SampleSegmentT = 0.3f;
    for (float s = 0.0f; s < SAMPLE_COUNT_INI_MULTI_SCAT; s += 1.0f)
    {
        // Exact difference, important for accuracy of multiple scattering
        float NewT = tMax * (s + SampleSegmentT) / SAMPLE_COUNT_INI_MULTI_SCAT;
        dt = NewT - t;
        t = NewT;
        float3 P = WorldPos + t * WorldDir;

        // Evaluate the sample transmittance
        MediumSampleRGB medium = SampleMediumRGB(P);
        const float3 SampleOpticalDepth = medium.extinction * dt;
        const float3 SampleTransmittance = exp(-SampleOpticalDepth);

        float pHeight = length(P);
        const float3 UpVector = P / pHeight;
        float SunZenithCosAngle = dot(SunDir, UpVector);
        float2 uv;
        LutTransmittanceParamsToUv(pHeight, SunZenithCosAngle, uv);
        float3 TransmittanceToSun = transmittanceLUTTexture.SampleLevel(samplerLinearClamp, uv, 0).rgb;

        float3 PhaseTimesScattering = medium.scattering * uniformPhase;

        // Earth shadow 
        float tEarth = ray_sphere_intersect_nearest(P, SunDir, earthO + PLANET_RADIUS_OFFSET * UpVector, _BottomRadius);
        float earthShadow = tEarth >= 0.0f ? 0.0f : 1.0f;

        float3 S = (earthShadow * TransmittanceToSun * PhaseTimesScattering);

        float3 MS = medium.scattering * 1;
        float3 MSint = (MS - MS * SampleTransmittance) / medium.extinction;
        multiScat += throughput * MSint;

        // See slide 28 at http://www.frostbite.com/2015/08/physically-based-unified-volumetric-rendering-in-frostbite/ 
        float3 Sint = (S - S * SampleTransmittance) / medium.extinction;    // integrate along the current step segment 
        luminance += throughput * Sint;                                                     // accumulate and also take into account the transmittance from previous steps
        throughput *= SampleTransmittance;

        // Move on to the next step
        tPrev = t;
    }

    if (tMax == tBottom && tBottom > 0.0)
    {
        // Account for bounced light off the earth
        float3 P = WorldPos + tBottom * WorldDir;
        float pHeight = length(P);

        const float3 UpVector = P / pHeight;
        float SunZenithCosAngle = dot(SunDir, UpVector);
        float2 uv;
        LutTransmittanceParamsToUv(pHeight, SunZenithCosAngle, uv);
        float3 TransmittanceToSun = transmittanceLUTTexture.SampleLevel(samplerLinearClamp, uv, 0).rgb;

        const float NdotL = saturate(dot(normalize(UpVector), normalize(SunDir)));
        luminance += 1.0f * throughput * NdotL * _GroundAlbedo / PI;
    }
}

float3 GetMultipleScattering(float3 scattering, float3 extinction, float3 worldPos, float viewZenithCosAngle, Texture2D<float4> multiScatteringLUTTexture, SamplerState samplerLinearClamp)
{
    float2 uv = saturate(float2(viewZenithCosAngle*0.5f + 0.5f, (length(worldPos) - _BottomRadius) / (_TopRadius - _BottomRadius)));
    uv = float2(fromUnitToSubUvs(uv.x, MULTI_SCATTERING_TEXTURE_RESOLUTION), fromUnitToSubUvs(uv.y, MULTI_SCATTERING_TEXTURE_RESOLUTION));
    return multiScatteringLUTTexture.SampleLevel(samplerLinearClamp, uv, 0).xyz;
}

void IntegrateLuminanceThroughput(float3 WorldPos, float3 WorldDir, float3 SunDir, float occluderDistance, uint maxSampleCount
    , Texture2D<float4> transmittanceLUTTexture, Texture2D<float4> multiScatteringLUTTexture, SamplerState samplerLinearClamp
    , out float3 inScattering, out float3 throughput)
{
    // Initialize the outputs
    inScattering = 0.0f;
    throughput = 1.0;

    // Compute next intersection with atmosphere or ground 
    float tBottom = ray_sphere_intersect_nearest(WorldPos, WorldDir, _PlanetCenterSky, _BottomRadius);
    float tTop = ray_sphere_intersect_nearest(WorldPos, WorldDir, _PlanetCenterSky, _TopRadius);
    float tMax = 0.0f;
    if (tBottom < 0.0f)
    {
        if (tTop < 0.0f)
            return;
        else
            tMax = tTop;
    }
    else if (tTop > 0.0f)
        tMax = min(tTop, tBottom);

    // Clamp it to the occluder distance
    if (occluderDistance > 0.0f)
        tMax = min(tMax, occluderDistance);
    tMax = min(tMax, 9000000.0f);

    // Sample count
    #if defined(SKY_VIEW_EVAL)
    float SampleCount = SAMPLE_COUNT_INI_SKY_VIEW;
    float SampleCountFloor = SAMPLE_COUNT_INI_SKY_VIEW;
    #else
    float SampleCount = lerp(4, maxSampleCount, saturate(tMax*0.01));
    float SampleCountFloor = floor(SampleCount);
    #endif
    float tMaxFloor = tMax * SampleCountFloor / SampleCount;  // rescale tMax to map to the last entire step segment.
    float dt = tMax / SampleCount;

    // Phase functions
    const float uniformPhase = 1.0 / (4.0 * PI);
    const float3 wi = SunDir;
    const float3 wo = WorldDir;
    float cosTheta = dot(wi, wo);
    float MiePhaseValue = CornetteShanksMiePhaseFunction(_MiePhaseG, -cosTheta); // mnegate cosTheta because due to WorldDir being a "in" direction. 
    float RayleighPhaseValue = RayleighPhase(cosTheta);

    // Ray march the atmosphere to integrate optical depth
    float3 OpticalDepth = 0.0;
    float t = 0.0f;
    float tPrev = 0.0;
    const float SampleSegmentT = 0.3f;
    for (float s = 0.0f; s < SampleCount; s += 1.0f)
    {
        // More expenssive but artefact free
        float t0 = (s) / SampleCountFloor;
        float t1 = (s + 1.0f) / SampleCountFloor;

        // Non linear distribution of sample within the range.
        t0 = t0 * t0;
        t1 = t1 * t1;

        // Make t0 and t1 world space distances.
        t0 = tMaxFloor * t0;
        if (t1 > 1.0)
            t1 = tMax;
        else
            t1 = tMaxFloor * t1;

        t = t0 + (t1 - t0)*SampleSegmentT;
        dt = t1 - t0;

        float3 P = WorldPos + t * WorldDir;

        MediumSampleRGB medium = SampleMediumRGB(P);
        const float3 SampleOpticalDepth = medium.extinction * dt;
        const float3 SampleTransmittance = exp(-SampleOpticalDepth);

        float pHeight = length(P);
        const float3 UpVector = P / pHeight;
        float SunZenithCosAngle = dot(SunDir, UpVector);
        float2 uv;
        LutTransmittanceParamsToUv(pHeight, SunZenithCosAngle, uv);
        float3 TransmittanceToSun = transmittanceLUTTexture.SampleLevel(samplerLinearClamp, uv, 0).xyz;
        float3 PhaseTimesScattering = medium.scatteringMie * MiePhaseValue + medium.scatteringRay * RayleighPhaseValue;

        // Earth shadow 
        float tEarth = ray_sphere_intersect_nearest(P, SunDir, _PlanetCenterSky + PLANET_RADIUS_OFFSET * UpVector, _BottomRadius);
        float earthShadow = tEarth >= 0.0f ? 0.0f : 1.0f;

        // Dual scattering for multi scattering 
        float3 multiScatteredLuminance = GetMultipleScattering(medium.scattering, medium.extinction, P, SunZenithCosAngle, multiScatteringLUTTexture, samplerLinearClamp);

        float3 S = (earthShadow * TransmittanceToSun * PhaseTimesScattering + multiScatteredLuminance * medium.scattering);

        // See slide 28 at http://www.frostbite.com/2015/08/physically-based-unified-volumetric-rendering-in-frostbite/ 
        float3 Sint = (S - S * SampleTransmittance) / medium.extinction;    // integrate along the current step segment 
        inScattering += throughput * Sint;                                                     // accumulate and also take into account the transmittance from previous steps
        throughput *= SampleTransmittance;

        // Move on to the next step
        tPrev = t;
    }

    inScattering *= SKY_INTENSITY_MUTLIPLIER;
}

float3 GetSunLuminance(float3 WorldPos, float3 WorldDir, float PlanetRadius)
{
    if (dot(WorldDir, _SunDirection) > cos(0.5*0.505* PI / 180.0))
    {
        float t = ray_sphere_intersect_nearest(WorldPos, WorldDir, _PlanetCenterSky, PlanetRadius);
        if (t < 0.0f) // no intersection
        {
            const float3 SunLuminance = 1000000.0; // arbitrary. But fine, not use when comparing the models
            return SunLuminance;
        }
    }
    return 0;
}

bool MoveToTopAtmosphere(inout float3 WorldPos, in float3 WorldDir)
{
    float viewHeight = length(WorldPos);
    if (viewHeight > _TopRadius)
    {
        float tTop = ray_sphere_intersect_nearest(WorldPos, WorldDir, _PlanetCenterSky, _TopRadius);
        if (tTop >= 0.0f)
        {
            float3 UpVector = WorldPos / viewHeight;
            float3 UpOffset = UpVector * -PLANET_RADIUS_OFFSET;
            WorldPos = WorldPos + WorldDir * tTop + UpOffset;
        }
        else
        {
            // Ray is not intersecting the atmosphere
            return false;
        }
    }
    return true; // ok to start tracing
}

#if !defined(FP64_UNSUPPORTED)
bool MoveToTopAtmosphere(inout double3 WorldPos, in double3 WorldDir)
{   
    double topRadius2 = _TopRadius * _TopRadius;
    double viewHeight2 = dot_double(WorldPos, WorldPos);
    if (viewHeight2 > topRadius2)
    {
        double tTop = ray_sphere_intersect_nearest_d(WorldPos, WorldDir, _PlanetCenterSky, _TopRadius);
        if (tTop >= 0.0)
        {
            double3 UpVector = WorldPos * invsqrt_double(viewHeight2);
            double3 UpOffset = UpVector * -PLANET_RADIUS_OFFSET;
            WorldPos = WorldPos + WorldDir * tTop + UpOffset;
        }
        else
        {
            // Ray is not intersecting the atmosphere
            return false;
        }
    }
    return true; // ok to start tracing
}
#endif

float3 EvaluateSunLightColor(float3 upVector, float elevation, float3 sunDirection, Texture2D<float4> transmittanceLUTTexture, SamplerState samplerLinearClamp)
{
    float earthShadow = dot(upVector, sunDirection) < 0.0 ? (ray_sphere_intersect_nearest(upVector * elevation, sunDirection, _PlanetCenterSky, _BottomRadius) != -1.0 ? 0.0 : 1.0) : 1.0;
    float viewZenithCosAngle = dot(sunDirection, upVector);
    float2 uv;
    LutTransmittanceParamsToUv(elevation, viewZenithCosAngle, uv);
    return transmittanceLUTTexture.SampleLevel(samplerLinearClamp, uv, 0).rgb * earthShadow;
}

#endif // SKY_UTILITIES_H
