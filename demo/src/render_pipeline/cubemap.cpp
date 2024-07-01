// Project includes
#include "render_pipeline/cubemap.h"
#include "tools/texture_utils.h"
#include "tools/shader_utils.h"
#include "tools/security.h"
#include "graphics/dx12_backend.h"

// CBVs
#define GLOBAL_CB_BINDING_SLOT CBV_SLOT(0)
#define RENDER_CUBEMAP_CBV_COUNT 1

// SRVs
#define CUBEMAP_TEXTURE_BINDING_SLOT SRV_SLOT(0)
#define RENDER_CUBEMAP_SRV_COUNT 1

// Samplers
#define CUBEMAP_SAMPLER_BINDING_SLOT SPL_SLOT(0)
#define RENDER_CUBEMAP_SAMPLER_COUNT 1

Cubemap::Cubemap()
{
}
Cubemap::~Cubemap()
{
}

void Cubemap::initialize(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, const std::string& textureLibrary)
{
    // Keep track of the device
    m_Device = device;

    // Initialize the sampler
    m_LinearClampSampler = SamplerDescriptor({ FilterMode::Linear, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp });

    // Load the texture to a buffer
    const std::string& imagePath = textureLibrary + "/milky_way/milky_way.png";

    // Texture descriptor
    TextureDescriptor desc;
    desc.type = TextureType::Tex2D;
    desc.depth = 1;
    desc.mipCount = 1;

    // Load the image
    GraphicsBuffer imageBuffer = load_image_to_graphics_buffer_png(device, imagePath.c_str(), desc.width, desc.height, desc.format);
    
    // Create the texture
    m_SpaceTexture = d3d12::graphics_resources::create_texture(device, desc);

    // Copy the buffer to a texture
    d3d12::command_buffer::reset(cmdB);
    d3d12::command_buffer::copy_buffer_into_texture(cmdB, imageBuffer, 0, m_SpaceTexture, 0);
    d3d12::command_buffer::close(cmdB);
    d3d12::command_queue::execute_command_buffer(cmdQ, cmdB);
    d3d12::command_queue::flush(cmdQ);

    // Destroy the graphics buffer
    d3d12::graphics_resources::destroy_graphics_buffer(imageBuffer);
}

void Cubemap::release()
{
    d3d12::graphics_pipeline::destroy_graphics_pipeline(m_CubemapGP);
    d3d12::graphics_resources::destroy_texture(m_SpaceTexture);
}

void Cubemap::reload_shaders(const std::string& shaderLibrary)
{
    GraphicsPipelineDescriptor gpd;
    gpd.filename = shaderLibrary + "/Sky/RenderCubemap.graphics";
    gpd.includeDirectories.push_back(shaderLibrary);
    gpd.cbvCount = RENDER_CUBEMAP_CBV_COUNT;
    gpd.srvCount = RENDER_CUBEMAP_SRV_COUNT;
    gpd.samplerCount = RENDER_CUBEMAP_SAMPLER_COUNT;
    compile_and_replace_graphics_pipeline(m_Device, gpd, m_CubemapGP);
}

void Cubemap::render_cubemap(CommandBuffer cmd, ConstantBuffer globalCB)
{
    d3d12::command_buffer::start_section(cmd, "Render Cubemap");
    {
        // CBVs
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_CubemapGP, GLOBAL_CB_BINDING_SLOT, globalCB);

        // SRVs
        d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmd, m_CubemapGP, CUBEMAP_TEXTURE_BINDING_SLOT, m_SpaceTexture);

        // Sampler
        d3d12::command_buffer::set_graphics_pipeline_sampler(cmd, m_CubemapGP, CUBEMAP_SAMPLER_BINDING_SLOT, m_LinearClampSampler);

        // Draw a full screen quad
        d3d12::command_buffer::draw_procedural(cmd, m_CubemapGP, 1, 1);
    }
    d3d12::command_buffer::end_section(cmd);
}