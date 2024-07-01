#ifndef EARTH_UTILITIES
#define EARTH_UTILITIES

float3 evaluate_earth_lighting(REAL3_DP positionOPS, float3 positionRWS, bool attenuation = true)
{
	// Evaluate the planet space position
    REAL3_DP posNPS = evaluate_normalized_planet_space(positionOPS, _PlanetRadius);

    // Evaluate the sample UV
    REAL2_DP sampleNUV = project_position_to_disk(posNPS);

    // Compute the local vector
    float3x3 localFrame = get_local_frame((float3)posNPS, (float2)sampleNUV);

    // Evaluate the displacements for this vertex
    REAL2_DP sampleUV = sampleNUV * _PlanetRadius;

    // Data required for the lighting
    BSDFData bsdfData;

    // Evaluate the distance to camera
    float distanceToCamera = length(positionRWS);

    // Evaluate the normal and roughness
    bsdfData.normalWS = EvaluateNormal(sampleUV, localFrame, distanceToCamera, _PatchSize, _PatchFlags, attenuation);

    // Lerp to the vertical normal near the equator to avoid a seam
    float3 targetNormal = normalize(float3(bsdfData.normalWS.x, 0, bsdfData.normalWS.z));
    bsdfData.normalWS = lerp(targetNormal, bsdfData.normalWS, saturate(abs((float)posNPS.y) / 1e-7));

    // Evaluate the roughness
    bsdfData.roughness = EvaluateRoughness(distanceToCamera, _PatchSize, _PatchRoughness);

    // View vector
    bsdfData.viewWS = -positionRWS / max(distanceToCamera, 0.00001);

    // Check if we need to patch the normal
    float NdotV = sanitize_normal(bsdfData.normalWS, bsdfData.viewWS, positionRWS);

    // Compute the planet normal
    float3 planetUpVector = normalize((float3)positionOPS);

    // Evaluate the reflection vector and attenuation
    bsdfData.reflectedWS = reflect(-bsdfData.viewWS, bsdfData.normalWS);
    bsdfData.reflectionAttenuation = 1.0f;
    AdjustReflectionVector(bsdfData.reflectedWS, planetUpVector, bsdfData.reflectionAttenuation);

    // Simple diffuse color for now
    bsdfData.diffuseColor = float3(0.0, 0.02, 0.04);

    // Evaluate the displaced planet space position
    REAL3_DP positionPS = REAL3_DP(positionRWS) + _CameraPosition - _PlanetCenter;

    // Evaluate the light color
    float3 lightColor = EvaluateSunLightColor(planetUpVector, length((float3)positionPS) / 1000.0, _SunDirection, _TransmittanceLUTTexture, sky_sampler_linear_clamp);

    // Evaluate the direct lighting
    float3 lighting = EvaluateDirectLighting(bsdfData, NdotV, _SunDirection, lightColor);
    
    // Add indirect lighting
    lighting += EvaluateIndirectLighting(bsdfData, positionPS, _SunDirection, _PlanetRadius, _TransmittanceLUTTexture, _MultiScatteringLUTTexture, sky_sampler_linear_clamp);

    // return the lighting
    return lighting;
}

#endif // EARTH_UTILITIES