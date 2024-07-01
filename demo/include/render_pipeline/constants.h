#pragma once

// Project includes
#include "math/types.h"

// Camera properties
const float g_CameraFOV = 20.0f * DEG_TO_RAD;

// Earth data
const float g_EarthRadius = 6371000.0;
const double3 g_EarthCenter = { 0.0, 0.0, 0.0 };
const float g_EarthImpostorToggle = g_EarthRadius * 1.1f;
const float g_EarthTriangleSize = 60.0f;

// Moon data
const float g_MoonRadius = 1737400.0;
const double3 g_MoonCenter = double3({ 30000000.0, 12000000.0, 0.0 });
const float g_MoonImpostorToggle = g_MoonRadius * 2.0f;
const float g_MoonTriangleSize = 60.0f;

// Convert kmh to mpsec
const float g_KMPerHourToMPerSec = 1.0f / 3.6f;

// Water simulation
const uint32_t g_WaterSimResolution = 256;
const uint32_t g_WaterSimBandCount = 4;
const float4 g_WaterSimPatchSize = float4({ 5.0f * 216.0f * PI * PI * PI, 5 * PI * PI * 36.0f, 5.0f * PI * 6, 5.0f });
const float4 g_WaterSimPatchRoughness = float4({ 0.08, 0.04, 0.02, 0.002 });

// Set of materials
#define UNUSED_MATERIAL 0
#define EARTH_MATERIAL 1
#define MOON_MATERIAL 2

// Other constants
#define LEB_MATRIX_CACHE_SIZE 5