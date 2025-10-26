#pragma once

// Project includes
#include "graphics/types.h"

// System includes
#include <string>

class MipBuilder
{
public:
    // Cst & Dst
    MipBuilder();
    ~MipBuilder();

    // Init & release
    void initialize(GraphicsDevice device);
    void release();

    // Shader reload
    void reload_shaders(const std::string& shaderLibrary);

    // Build mips for a texture
    void build_mips(CommandBuffer cmd, Texture targetTexture);

private:
    // Graphics device
    GraphicsDevice m_Device = 0;

    // Target shader
    ComputeShader m_BuildMipsCS = 0;
};
