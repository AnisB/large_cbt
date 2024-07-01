#pragma once

// Project includes
#include "graphics/types.h"
#include "water/water_simulation.h"
#include "water/water_data.h"

class MoonMaterial
{
public:
    // Cst & Dst
    MoonMaterial();
    ~MoonMaterial();

    // Init and release
    void initialize(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmd, const std::string& textureLibrary);
    void release();

    // Reload the shaders
    void reload_shaders(const std::string& shaderLibrary);

    // Prepare the rendering
    void prepare_rendering(CommandBuffer cmd, ConstantBuffer planetCB);

    // Update the constant buffers
    void update_constant_buffers(CommandBuffer cmd);

    // UI of the moon material
    void render_ui();

    // Albedo texture
    Texture get_albedo_texture() const { return m_AlbedoTexture; }

    // Elevation texture
    Texture get_elevation_texture() const { return m_ElevationTexture; }

    // Surface gradient texture
    Texture get_elevation_sg_texture() const { return m_ElevationSGTexture; }

    // Detail texture
    Texture get_detail_texture() const { return m_DetailTexture; }

    // Detail sg texture
    Texture get_detail_sg_texture() const { return m_DetailSGTexture; }

    ConstantBuffer get_moon_cb() const { return m_MoonCB; }

private:
    // Graphcis device
    GraphicsDevice m_Device = 0;

    // Albedo
    Texture m_AlbedoTexture = 0;
    uint32_t m_AlbedoWidth = 0, m_AlbedoHeight = 0;

    // Elevation texture
    Texture m_ElevationTexture = 0;
    uint32_t m_ElevationWidth = 0, m_ElevationHeight = 0;

    // Elevation surface gradient
    Texture m_ElevationSGTexture = 0;
    uint32_t m_ElevationSGWidth = 0, m_ElevationSGHeight = 0;

    // Compute shaders
    ComputeShader m_EvaluateSurfaceGradientCS = 0;
    ComputeShader m_EvaluateDetailSlopeCS = 0;
    ConstantBuffer m_MoonCB = 0;

    // Detail
    Texture m_DetailTexture = 0;
    Texture m_DetailSGTexture = 0;
    uint32_t m_DetailWidth = 0, m_DetailHeight = 0;

    // Detail properties
    float m_PatchSize = 0.0;
    float m_PatchAmplitude = 0.0;
    int32_t m_NumOctaves = 0;
    bool m_Attenuation = 0;
};