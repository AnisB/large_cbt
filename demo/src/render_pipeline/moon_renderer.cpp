// Project includes
#include "graphics/dx12_backend.h"
#include "render_pipeline/moon_renderer.h"
#include "render_pipeline/stencil_usage.h"
#include "tools/shader_utils.h"
#include "tools/security.h"

MoonRenderer::MoonRenderer()
{
}

MoonRenderer::~MoonRenderer()
{
}

void MoonRenderer::initialize(GraphicsDevice device, const std::string& textureLibrary)
{
    // Keep track of the device
    m_Device = device;

    // Initialize the samplers
    m_LinearClamp = { FilterMode::Linear, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp };
    m_LinearWrap = { FilterMode::Linear, SamplerMode::Wrap, SamplerMode::Wrap, SamplerMode::Wrap };
}

void MoonRenderer::release()
{
    d3d12::graphics_pipeline::destroy_graphics_pipeline(m_ImpostorGP);
    d3d12::graphics_pipeline::destroy_graphics_pipeline(m_SolidWireGP);
    d3d12::compute_shader::destroy_compute_shader(m_MaterialCS);
    d3d12::graphics_pipeline::destroy_graphics_pipeline(m_VisibilityGP);
}

void MoonRenderer::reload_shaders(const std::string& shaderLibrary)
{
    // Initialize the close rendering pipeline
    {
        GraphicsPipelineDescriptor gpd;
        gpd.includeDirectories.push_back(shaderLibrary);
        gpd.filename = shaderLibrary + "\\" + "Visibility\\VisibilityPass.graphics";
        gpd.cbvCount = 2;
        gpd.srvCount = 2;
        gpd.rtFormat[0] = TextureFormat::R16G16B16A16_UInt;

        // Depth
        gpd.depthStencilState.enableDepth = true;
        gpd.depthStencilState.depthtest = DepthTest::Less;
        gpd.depthStencilState.depthStencilFormat = TextureFormat::Depth32Stencil8;

        // Stencil
        gpd.depthStencilState.enableStencil = true;
        gpd.depthStencilState.stencilWriteMask = (uint32_t)StencilUsage::Foreground | (uint32_t)StencilUsage::SkyOccluder;
        gpd.depthStencilState.stencilTest = StencilTest::Always;
        gpd.depthStencilState.stencilRef = (uint32_t)StencilUsage::Foreground | (uint32_t)StencilUsage::SkyOccluder;
        gpd.depthStencilState.stencilOperation = StencilOp::Replace;
        compile_and_replace_graphics_pipeline(m_Device, gpd, m_VisibilityGP);

        // Material kernel
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\Moon\\MaterialPass.compute";
        csd.cbvCount = 4;
        csd.srvCount = 6;
        csd.uavCount = 1;
        csd.samplerCount = 2;
        csd.kernelname = "EvaluateMaterial";
        compile_and_replace_compute_shader(m_Device, csd, m_MaterialCS);
    }

    // Initialize the solidwire pipeline
    {
        GraphicsPipelineDescriptor gpd;
        gpd.includeDirectories.push_back(shaderLibrary);
        gpd.filename = shaderLibrary + "\\Moon\\SolidWire.graphics";
        gpd.geometryKernelName = "geom";
        gpd.cbvCount = 4;
        gpd.srvCount = 6;
        gpd.samplerCount = 2;
        gpd.depthStencilState.enableDepth = true;
        gpd.depthStencilState.depthtest = DepthTest::Less;
        gpd.depthStencilState.depthStencilFormat = TextureFormat::Depth32Stencil8;
        gpd.depthStencilState.enableStencil = true;
        gpd.depthStencilState.stencilWriteMask = (uint32_t)StencilUsage::Foreground | (uint32_t)StencilUsage::SkyOccluder;
        gpd.depthStencilState.stencilTest = StencilTest::Always;
        gpd.depthStencilState.stencilRef = (uint32_t)StencilUsage::Foreground | (uint32_t)StencilUsage::SkyOccluder;
        gpd.depthStencilState.stencilOperation = StencilOp::Replace;
        gpd.rtFormat[0] = TextureFormat::R16G16B16A16_Float;
        compile_and_replace_graphics_pipeline(m_Device, gpd, m_SolidWireGP);
    }

    // Initialize the distant rendering pipeline
    {
        // Create the compute shaders
        GraphicsPipelineDescriptor gpd;
        gpd.includeDirectories.push_back(shaderLibrary);
        gpd.filename = shaderLibrary + "\\Moon\\ForwardPassImpostor.graphics";
        gpd.cbvCount = 3;
        gpd.srvCount = 2;
        gpd.samplerCount = 1;

        // Depth
        gpd.depthStencilState.enableDepth = true;
        gpd.depthStencilState.depthtest = DepthTest::LEqual;
        gpd.depthStencilState.depthStencilFormat = TextureFormat::Depth32Stencil8;

        // Stencil
        gpd.depthStencilState.enableStencil = true;
        gpd.depthStencilState.stencilWriteMask = (uint32_t)StencilUsage::Background | (uint32_t)StencilUsage::SkyOccluder;
        gpd.depthStencilState.stencilReadMask = (uint32_t)StencilUsage::Background;
        gpd.depthStencilState.stencilTest = StencilTest::Greater;
        gpd.depthStencilState.stencilRef = (uint32_t)StencilUsage::Background;
        gpd.depthStencilState.stencilOperation = StencilOp::Replace;

        compile_and_replace_graphics_pipeline(m_Device, gpd, m_ImpostorGP);
    }
}

void MoonRenderer::render_visibility_buffer(CommandBuffer cmd, RenderTexture visibilityBuffer, RenderTexture depthTexture, const Planet& planet, ConstantBuffer globalCB)
{
    // Get the mesh
    const CBTMesh& targetMesh = planet.get_cbt_mesh();
    d3d12::command_buffer::start_section(cmd, "Render Moon Visibility Buffer");
    {
        // Set the visibility buffer for the draw
        d3d12::command_buffer::set_render_texture(cmd, visibilityBuffer, depthTexture);

        // Constant buffers
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_VisibilityGP, 0, globalCB);
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_VisibilityGP, 1, planet.get_geometry_cb());

        // SRVs
        d3d12::command_buffer::set_graphics_pipeline_buffer_srv(cmd, m_VisibilityGP, 0, targetMesh.currentVertexBuffer);
        d3d12::command_buffer::set_graphics_pipeline_buffer_srv(cmd, m_VisibilityGP, 1, targetMesh.indexedBisectorBuffer);

        // Draw the geometry
        d3d12::command_buffer::draw_procedural_indirect(cmd, m_VisibilityGP, targetMesh.indirectDrawBuffer);
    }
    d3d12::command_buffer::end_section(cmd);
}

void MoonRenderer::render_material(CommandBuffer cmd, RenderTexture colorBuffer, RenderTexture visibilityBuffer, RenderTexture depthTexture, const MoonMaterial& moonMaterial, const Planet& planet, ConstantBuffer globalCB)
{
    // Get the mesh
    const CBTMesh& targetMesh = planet.get_cbt_mesh();

    // Get the size of the texture
    uint32_t width, height, depth;
    d3d12::graphics_resources::render_texture_dimensions(depthTexture, width, height, depth);

    // Render the mesh
    d3d12::command_buffer::start_section(cmd, "Render Moon Material");
    {
        // Constant buffers
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_MaterialCS, 0, globalCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_MaterialCS, 1, planet.get_planet_cb());
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_MaterialCS, 2, moonMaterial.get_moon_cb());

        // SRVs
        d3d12::command_buffer::set_compute_shader_buffer_srv(cmd, m_MaterialCS, 0, targetMesh.currentVertexBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_srv(cmd, m_MaterialCS, 1, targetMesh.currentDisplacementBuffer);
        d3d12::command_buffer::set_compute_shader_texture_srv(cmd, m_MaterialCS, 2, moonMaterial.get_albedo_texture());
        d3d12::command_buffer::set_compute_shader_texture_srv(cmd, m_MaterialCS, 3, moonMaterial.get_elevation_sg_texture());
        d3d12::command_buffer::set_compute_shader_texture_srv(cmd, m_MaterialCS, 4, moonMaterial.get_detail_sg_texture());
        d3d12::command_buffer::set_compute_shader_render_texture_srv(cmd, m_MaterialCS, 5, visibilityBuffer);

        // UAVs
        d3d12::command_buffer::set_compute_shader_render_texture_uav(cmd, m_MaterialCS, 0, colorBuffer);

        // Sampler
        d3d12::command_buffer::set_compute_shader_sampler(cmd, m_MaterialCS, 0, m_LinearWrap);
        d3d12::command_buffer::set_compute_shader_sampler(cmd, m_MaterialCS, 1, m_LinearWrap);

        // Draw the geometry
        d3d12::command_buffer::dispatch(cmd, m_MaterialCS, (width + 7) / 8, (height + 7) / 8, 1);

        // Set the visibility buffer for the draw
        d3d12::command_buffer::set_render_texture(cmd, colorBuffer, depthTexture);
    }
    d3d12::command_buffer::end_section(cmd);
}

void MoonRenderer::render_solid_wire(CommandBuffer cmd, RenderTexture colorBuffer, RenderTexture depthTexture, const MoonMaterial& moonMaterial, const Planet& planet, ConstantBuffer globalCB)
{
    // Get the mesh
    const CBTMesh& targetMesh = planet.get_cbt_mesh();

    // Render the mesh
    d3d12::command_buffer::start_section(cmd, "Render Moon SolidWire");
    {
        // Constant buffers
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_SolidWireGP, 0, globalCB);
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_SolidWireGP, 1, planet.get_planet_cb());
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_SolidWireGP, 2, moonMaterial.get_moon_cb());
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_SolidWireGP, 3, planet.get_update_cb());

        // SRVs
        d3d12::command_buffer::set_graphics_pipeline_buffer_srv(cmd, m_SolidWireGP, 0, targetMesh.currentVertexBuffer);
        d3d12::command_buffer::set_graphics_pipeline_buffer_srv(cmd, m_SolidWireGP, 1, targetMesh.currentDisplacementBuffer);
        d3d12::command_buffer::set_graphics_pipeline_buffer_srv(cmd, m_SolidWireGP, 2, targetMesh.indexedBisectorBuffer);
        d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmd, m_SolidWireGP, 3, moonMaterial.get_albedo_texture());
        d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmd, m_SolidWireGP, 4, moonMaterial.get_elevation_sg_texture());
        d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmd, m_SolidWireGP, 5, moonMaterial.get_detail_sg_texture());

        // Sampler
        d3d12::command_buffer::set_graphics_pipeline_sampler(cmd, m_SolidWireGP, 0, m_LinearWrap);
        d3d12::command_buffer::set_graphics_pipeline_sampler(cmd, m_SolidWireGP, 1, m_LinearWrap);

        // Draw
        d3d12::command_buffer::draw_procedural_indirect(cmd, m_SolidWireGP, targetMesh.indirectDrawBuffer);
    }
    d3d12::command_buffer::end_section(cmd);
}

void MoonRenderer::render_planet_impostor(CommandBuffer cmd, const MoonMaterial& moonMaterial, const Planet& planet, ConstantBuffer globalCB, bool skyOccluder)
{
    // Adjust the stencil ref
    uint8_t stencilRef = skyOccluder ? ((uint32_t)StencilUsage::SkyOccluder | (uint32_t)StencilUsage::Background) : (uint32_t)StencilUsage::Background;
    d3d12::graphics_pipeline::set_stencil_ref(m_ImpostorGP, stencilRef);

    d3d12::command_buffer::start_section(cmd, "Render Moon (impostor)");
    {
        // Constant buffers
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_ImpostorGP, 0, globalCB);
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_ImpostorGP, 1, planet.get_planet_cb());

        // SRVs
        d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmd, m_ImpostorGP, 0, moonMaterial.get_albedo_texture());
        d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmd, m_ImpostorGP, 1, moonMaterial.get_elevation_sg_texture());

        // Samplers
        d3d12::command_buffer::set_graphics_pipeline_sampler(cmd, m_ImpostorGP, 0, m_LinearClamp);

        // Dispatch
        d3d12::command_buffer::draw_procedural(cmd, m_ImpostorGP, 1, 1);
    }
    d3d12::command_buffer::end_section(cmd);
}