// Project includes
#include "tools/mip_builder.h"
#include "tools/shader_utils.h"

// Project includes
#include "graphics/dx12_backend.h"

MipBuilder::MipBuilder()
{

}

MipBuilder::~MipBuilder()
{

}

void MipBuilder::initialize(GraphicsDevice device)
{
    // Keep track of the device
    m_Device = device;
}

void MipBuilder::release()
{
    d3d12::compute_shader::destroy_compute_shader(m_BuildMipsCS);
}

// Shader reload
void MipBuilder::reload_shaders(const std::string& shaderLibrary)
{
    // Create the shader
    ComputeShaderDescriptor csd;
    csd.includeDirectories.push_back(shaderLibrary);
    csd.filename = shaderLibrary + "\\BuildMips.compute";
    csd.uavCount = 5;
    csd.kernelname = "BuildMips";
    compile_and_replace_compute_shader(m_Device, csd, m_BuildMipsCS);
}

// Build mips for a texture
void MipBuilder::build_mips(CommandBuffer cmdB, Texture targetTexture)
{   
    // Get the sze of the texture
    uint32_t width, height, depth;
    d3d12::graphics_resources::texture_dimensions(targetTexture, width, height, depth);
    const uint32_t tileX = (width + 7) / 8;
    const uint32_t tileY = (height + 7) / 8;

    // UAVs
    d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_BuildMipsCS, 0, targetTexture, 0);
    d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_BuildMipsCS, 1, targetTexture, 1);
    d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_BuildMipsCS, 2, targetTexture, 2);
    d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_BuildMipsCS, 3, targetTexture, 3);
    d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_BuildMipsCS, 4, targetTexture, 4);

    // Dispatch
    d3d12::command_buffer::dispatch(cmdB, m_BuildMipsCS, tileX, tileY, 1);

    // Barrier
    d3d12::command_buffer::uav_barrier_texture(cmdB, targetTexture);
}