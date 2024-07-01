#pragma once

// Project includes
#include "graphics/types.h"
#include "graphics/descriptors.h"
#include "render_pipeline/camera.h"
#include "render_pipeline/planet.h"
#include "render_pipeline/sky.h"
#include "water/water_data.h"

class EarthRenderer
{
public:
    EarthRenderer();
    ~EarthRenderer();

    // Initialize & Release
    void initialize(GraphicsDevice device);
    void release();

    // Reload shaders
    void reload_shaders(const std::string& shaderLibrary);

    // Rendering the planet
    void render_visibility_buffer(CommandBuffer cmd, RenderTexture visibilityBuffer, RenderTexture depthTexture, const Planet& planet, ConstantBuffer globalCB);
    void render_material(CommandBuffer cmd, RenderTexture colorBuffer, RenderTexture visibilityBuffer, RenderTexture depthTexture, const Planet& planet, Texture sgTexture, const Sky& sky, ConstantBuffer globalCB, ConstantBuffer deformationCB);
    void render_solid_wire(CommandBuffer cmd, RenderTexture colorBuffer, RenderTexture depthTexture, const Planet& planet, Texture sgTexture, const Sky& sky, ConstantBuffer globalCB, ConstantBuffer deformationCB);
    void render_planet_impostor(CommandBuffer cmd, const Planet& planet, const WaterData& waterData, const Sky& sky, ConstantBuffer globalCB);

private:
    // General graphics
    GraphicsDevice m_Device = 0;

    // Impostor
    GraphicsPipeline m_ImpostorGP = 0;

    // Earth rendering
    GraphicsPipeline m_VisibilityGP = 0;
    ComputeShader m_MaterialCS = 0;
    GraphicsPipeline m_SolidWireGP = 0;

    // Samplers
    SamplerDescriptor m_LinearClamp = SamplerDescriptor();
    SamplerDescriptor m_LinearWrap = SamplerDescriptor();
};