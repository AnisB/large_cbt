#pragma once

// Project includes
#include "graphics/types.h"
#include "graphics/descriptors.h"

class Cubemap
{
public:
    Cubemap();
    ~Cubemap();

    // Initialize and release
    void initialize(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, const std::string& textureLibrary);
    void release();

    // Reload the shaders
    void reload_shaders(const std::string& shaderLibrary);

    // Render the cubemap to the currently bound render target
    void render_cubemap(CommandBuffer cmd, ConstantBuffer skyC);

    // Return the texture if needed
    Texture get_cubemap() { return m_SpaceTexture; }

private:
    // Graphics device
    GraphicsDevice m_Device = 0;

    // Resources
    Texture m_SpaceTexture = 0;
    SamplerDescriptor m_LinearClampSampler = {};

    // Shader
    GraphicsPipeline m_CubemapGP = 0;
};
