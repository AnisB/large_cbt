// Project includes
#include "graphics/dx12_backend.h"
#include "imgui/imgui.h"
#include "moon/moon_material.h"
#include "render_pipeline/constants.h"
#include "render_pipeline/constant_buffers.h"
#include "tools/shader_utils.h"
#include "tools/texture_utils.h"

// Cst & Dst
MoonMaterial::MoonMaterial()
{
}

MoonMaterial::~MoonMaterial()
{
}

void MoonMaterial::initialize(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, const std::string& textureLibrary)
{
    // Keep track of the device
    m_Device = device;

    // Variables that will be re-used
    TextureFormat format;
    TextureDescriptor descD;
    descD.type = TextureType::Tex2D;
    descD.depth = 1;
    descD.mipCount = 1;

    // List of resources to load
    const std::string& albedoTexture = textureLibrary + "/moon_textures/albedo.png";
    const std::string& elevationTexture = textureLibrary + "/moon_textures/elevation.tif";
    const std::string& simplexTexture = textureLibrary + "/moon_textures/simplex.png";

    // Create the runtime texture
    GraphicsBuffer albedoBuffer = load_image_to_graphics_buffer_png(device, albedoTexture.c_str(), m_AlbedoWidth, m_AlbedoHeight, format);
    descD.width = m_AlbedoWidth;
    descD.height = m_AlbedoHeight;
    descD.format = format;
    descD.mipCount = 5;
    descD.isUAV = true;
    m_AlbedoTexture = d3d12::graphics_resources::create_texture(device, descD);

    // Create the runtime texture
    GraphicsBuffer elevationBuffer = load_image_to_graphics_buffer_tiff(device, elevationTexture.c_str(), m_ElevationWidth, m_ElevationHeight, format);
    descD.width = m_ElevationWidth;
    descD.height = m_ElevationHeight;
    descD.mipCount = 5;
    descD.isUAV = true;
    descD.format = format;
    m_ElevationTexture = d3d12::graphics_resources::create_texture(device, descD);
    descD.format = TextureFormat::R16G16B16A16_Float;
    m_ElevationSGTexture = d3d12::graphics_resources::create_texture(device, descD);

    // Create the runtime texture
    GraphicsBuffer simplexBuffer = load_image_to_graphics_buffer_png(device, simplexTexture.c_str(), m_DetailWidth, m_DetailHeight, format);
    descD.width = m_DetailWidth;
    descD.height = m_DetailHeight;
    descD.format = format;
    descD.mipCount = 5;
    descD.isUAV = true;
    m_DetailTexture = d3d12::graphics_resources::create_texture(device, descD);
    descD.format = TextureFormat::R16G16_Float;
    m_DetailSGTexture = d3d12::graphics_resources::create_texture(device, descD);

    // Copy the resources
    d3d12::command_buffer::reset(cmdB);
    d3d12::command_buffer::copy_buffer_into_texture(cmdB, albedoBuffer, 0, m_AlbedoTexture, 0);
    d3d12::command_buffer::copy_buffer_into_texture(cmdB, elevationBuffer, 0, m_ElevationTexture, 0);
    d3d12::command_buffer::copy_buffer_into_texture(cmdB, simplexBuffer, 0, m_DetailTexture, 0);

    // TODO add the mip evaluation for these textures
    d3d12::command_buffer::close(cmdB);
    d3d12::command_queue::execute_command_buffer(cmdQ, cmdB);
    d3d12::command_queue::flush(cmdQ);

    // Destroy the buffers
    d3d12::graphics_resources::destroy_graphics_buffer(simplexBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(elevationBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(albedoBuffer);

    // Create the constant buffer and set it's value
    m_MoonCB = d3d12::graphics_resources::create_constant_buffer(device, sizeof(MoonCB), ConstantBufferType::Mixed);

    // Material properties
    m_PatchSize = 15000.0f;
    m_PatchAmplitude = 250.0f;
    m_NumOctaves = 4;
    m_Attenuation = true;
}

void MoonMaterial::release()
{
    // Shaders
    d3d12::compute_shader::destroy_compute_shader(m_EvaluateSurfaceGradientCS);
    d3d12::compute_shader::destroy_compute_shader(m_EvaluateDetailSlopeCS);

    // Constant buffers
    d3d12::graphics_resources::destroy_constant_buffer(m_MoonCB);

    // Detail
    d3d12::graphics_resources::destroy_texture(m_DetailSGTexture);
    d3d12::graphics_resources::destroy_texture(m_DetailTexture);

    // Elevation
    d3d12::graphics_resources::destroy_texture(m_ElevationSGTexture);
    d3d12::graphics_resources::destroy_texture(m_ElevationTexture);

    // Albedo
    d3d12::graphics_resources::destroy_texture(m_AlbedoTexture);
}

void MoonMaterial::reload_shaders(const std::string& shaderLibrary)
{
    // Deformation kernel
    ComputeShaderDescriptor csd;
    csd.includeDirectories.push_back(shaderLibrary);
    csd.filename = shaderLibrary + "\\EvaluateSurfaceGradients.compute";
    csd.cbvCount = 2;
    csd.srvCount = 1;
    csd.uavCount = 2;
    csd.samplerCount = 1;
    csd.kernelname = "EvaluateSurfaceGradient";
    compile_and_replace_compute_shader(m_Device, csd, m_EvaluateSurfaceGradientCS);
    csd.kernelname = "EvaluateDetailSlope";
    compile_and_replace_compute_shader(m_Device, csd, m_EvaluateDetailSlopeCS);
}

void MoonMaterial::prepare_rendering(CommandBuffer cmdB, ConstantBuffer planetCB)
{
    {
        // CBVs
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateSurfaceGradientCS, 0, m_MoonCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateSurfaceGradientCS, 1, planetCB);

        // SRVs
        d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_EvaluateSurfaceGradientCS, 0, m_ElevationTexture);

        // UAVs
        d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_EvaluateSurfaceGradientCS, 0, m_ElevationSGTexture);

        // Samplers
        SamplerDescriptor linearWrap = { FilterMode::Linear, SamplerMode::Wrap, SamplerMode::Mirror, SamplerMode::Wrap };
        d3d12::command_buffer::set_compute_shader_sampler(cmdB, m_EvaluateSurfaceGradientCS, 0, linearWrap);

        // Dispatch
        d3d12::command_buffer::dispatch(cmdB, m_EvaluateSurfaceGradientCS, (m_ElevationWidth + 7) / 8, (m_ElevationHeight + 7) / 8, 1);

        // Barrier
        d3d12::command_buffer::uav_barrier_texture(cmdB, m_ElevationSGTexture);
    }
    {
        // CBVs
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateDetailSlopeCS, 0, m_MoonCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateDetailSlopeCS, 1, planetCB);

        // SRVs
        d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_EvaluateDetailSlopeCS, 0, m_DetailTexture);

        // UAVs
        d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_EvaluateDetailSlopeCS, 1, m_DetailSGTexture);

        // Dispatch
        d3d12::command_buffer::dispatch(cmdB, m_EvaluateDetailSlopeCS, (m_DetailWidth + 7) / 8, (m_DetailHeight + 7) / 8, 1);

        // Barrier
        d3d12::command_buffer::uav_barrier_texture(cmdB, m_DetailSGTexture);
    }
}

void MoonMaterial::update_constant_buffers(CommandBuffer cmd)
{
    MoonCB moonCB;
    moonCB._ElevationTextureSize = { m_ElevationWidth, m_ElevationHeight };
    moonCB._DetailTextureSize = { m_DetailWidth, m_DetailHeight };
    moonCB._PatchSize = m_PatchSize;
    moonCB._PatchAmplitude = m_PatchAmplitude;
    moonCB._NumOctaves = m_NumOctaves;
    moonCB._Attenuation = m_Attenuation ? 1 : 0;
    d3d12::graphics_resources::set_constant_buffer(m_MoonCB, (char*)&moonCB, sizeof(MoonCB));
    d3d12::command_buffer::upload_constant_buffer(cmd, m_MoonCB);
}

void MoonMaterial::render_ui()
{
    ImGui::InputFloat("Patch Sizes", &m_PatchSize);
    ImGui::InputFloat("Patch Amplitudes", &m_PatchAmplitude);
    ImGui::SliderInt("Num Octaves", &m_NumOctaves, 0, 4);
    ImGui::Checkbox("Attenuation", &m_Attenuation);
}