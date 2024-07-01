#pragma once

// Project includes
#include "graphics/types.h"
#include "graphics/descriptors.h"
#include "render_pipeline/camera.h"
#include "render_pipeline/planet.h"
#include "render_pipeline/sky.h"
#include "moon/moon_material.h"

class MoonRenderer
{
public:
    MoonRenderer();
    ~MoonRenderer();

    // Initialize & Release
    void initialize(GraphicsDevice device, const std::string& textureLibrary);
    void release();

    // Reload shaders
    void reload_shaders(const std::string& shaderLibrary);

    // Rendering the planet
    void render_visibility_buffer(CommandBuffer cmd, RenderTexture visibilityBuffer, RenderTexture depthTexture, const Planet& planet, ConstantBuffer globalCB);
    void render_material(CommandBuffer cmd, RenderTexture colorBuffer, RenderTexture visibilityBuffer, RenderTexture depthTexture, const MoonMaterial& moonMaterial, const Planet& planet, ConstantBuffer globalCB);
    void render_solid_wire(CommandBuffer cmd, RenderTexture colorBuffer, RenderTexture depthTexture, const MoonMaterial& moonMaterial, const Planet& planet, ConstantBuffer globalCB);
    void render_planet_impostor(CommandBuffer cmd, const MoonMaterial& moonMaterial, const Planet& planet, ConstantBuffer globalCB, bool skyOccluder);

private:
    // General graphics
    GraphicsDevice m_Device = 0;

    // Impostor
    GraphicsPipeline m_ImpostorGP = 0;

    // Shaders
    GraphicsPipeline m_VisibilityGP = 0;
    ComputeShader m_MaterialCS = 0;
    GraphicsPipeline m_SolidWireGP = 0;

    // Samplers
    SamplerDescriptor m_LinearClamp = SamplerDescriptor();
    SamplerDescriptor m_LinearWrap = SamplerDescriptor();
};