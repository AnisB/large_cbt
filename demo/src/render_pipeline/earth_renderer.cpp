// Project includes
#include "graphics/dx12_backend.h"
#include "tools/security.h"
#include "render_pipeline/earth_renderer.h"
#include "render_pipeline/stencil_usage.h"
#include "tools/shader_utils.h"

// CBVs
#define GLOBAL_CB_BINDING_SLOT CBV_SLOT(0)
#define PLANET_CB_BINDING_SLOT CBV_SLOT(1)
#define SKY_ATMOSPHERE_BUFFER_SLOT CBV_SLOT(2)
#define DEFORMATION_CB_BINDING_SLOT CBV_SLOT(3)
#define UPDATE_CB_BINDING_SLOT CBV_SLOT(4)

EarthRenderer::EarthRenderer()
{
}

EarthRenderer::~EarthRenderer()
{
}

void EarthRenderer::initialize(GraphicsDevice device)
{
    // Keep track of the device
    m_Device = device;

    // Initialize the samplers
    m_LinearClamp = { FilterMode::Linear, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp };
    m_LinearWrap = { FilterMode::Linear, SamplerMode::Wrap, SamplerMode::Wrap, SamplerMode::Wrap };
}

void EarthRenderer::release()
{
    d3d12::graphics_pipeline::destroy_graphics_pipeline(m_SolidWireGP);
    d3d12::compute_shader::destroy_compute_shader(m_MaterialCS);
    d3d12::graphics_pipeline::destroy_graphics_pipeline(m_VisibilityGP);
    d3d12::graphics_pipeline::destroy_graphics_pipeline(m_ImpostorGP);
}

void EarthRenderer::reload_shaders(const std::string& shaderLibrary)
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
        gpd.depthStencilState.stencilWriteMask = (uint32_t)StencilUsage::Foreground | (uint32_t)StencilUsage::SkyHost;
        gpd.depthStencilState.stencilTest = StencilTest::Always;
        gpd.depthStencilState.stencilRef = (uint32_t)StencilUsage::Foreground | (uint32_t)StencilUsage::SkyHost;
        gpd.depthStencilState.stencilOperation = StencilOp::Replace;
        compile_and_replace_graphics_pipeline(m_Device, gpd, m_VisibilityGP);

        // Material kernel
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\Earth\\MaterialPass.compute";
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
        gpd.filename = shaderLibrary + "\\Earth\\SolidWire.graphics";
        gpd.geometryKernelName = "geom";
        gpd.cbvCount = 5;
        gpd.srvCount = 6;
        gpd.samplerCount = 2;
        gpd.depthStencilState.enableDepth = true;
        gpd.depthStencilState.depthtest = DepthTest::Less;
        gpd.depthStencilState.depthStencilFormat = TextureFormat::Depth32Stencil8;
        gpd.depthStencilState.enableStencil = true;
        gpd.depthStencilState.stencilWriteMask = (uint32_t)StencilUsage::Foreground | (uint32_t)StencilUsage::SkyHost;
        gpd.depthStencilState.stencilTest = StencilTest::Always;
        gpd.depthStencilState.stencilRef = (uint32_t)StencilUsage::Foreground | (uint32_t)StencilUsage::SkyHost;
        gpd.depthStencilState.stencilOperation = StencilOp::Replace;
        gpd.rtFormat[0] = TextureFormat::R16G16B16A16_Float;
        compile_and_replace_graphics_pipeline(m_Device, gpd, m_SolidWireGP);
    }

    // Initialize the distant rendering pipeline
    {
        // Create the compute shaders
        GraphicsPipelineDescriptor gpd;
        gpd.includeDirectories.push_back(shaderLibrary);
        gpd.filename = shaderLibrary + "\\" + "Earth\\ForwardPassImpostor.graphics";
        gpd.cbvCount = 4;
        gpd.srvCount = 2;
        gpd.samplerCount = 1;

        // Depth
        gpd.depthStencilState.enableDepth = true;
        gpd.depthStencilState.depthtest = DepthTest::LEqual;
        gpd.depthStencilState.depthStencilFormat = TextureFormat::Depth32Stencil8;

        // Stencil
        gpd.depthStencilState.enableStencil = true;
        gpd.depthStencilState.stencilWriteMask = (uint32_t)StencilUsage::Background | (uint32_t)StencilUsage::SkyHost;
        gpd.depthStencilState.stencilTest = StencilTest::Greater;
        gpd.depthStencilState.stencilRef = (uint32_t)StencilUsage::Background | (uint32_t)StencilUsage::SkyHost;
        gpd.depthStencilState.stencilOperation = StencilOp::Replace;
        compile_and_replace_graphics_pipeline(m_Device, gpd, m_ImpostorGP);
    }
}

void EarthRenderer::render_visibility_buffer(CommandBuffer cmd, RenderTexture visibilityBuffer, RenderTexture depthTexture, const Planet& planet, ConstantBuffer globalCB)
{
    // Get the mesh
    const CBTMesh& targetMesh = planet.get_cbt_mesh();
    d3d12::command_buffer::start_section(cmd, "Render Earth Visibility Buffer");
    {
        // Set the visibility buffer for the draw
        d3d12::command_buffer::set_render_texture(cmd, visibilityBuffer, depthTexture);

        // Constant buffers
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_VisibilityGP, GLOBAL_CB_BINDING_SLOT, globalCB);
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_VisibilityGP, PLANET_CB_BINDING_SLOT, planet.get_geometry_cb());

        // SRVs
        d3d12::command_buffer::set_graphics_pipeline_buffer_srv(cmd, m_VisibilityGP, 0, targetMesh.currentVertexBuffer);
        d3d12::command_buffer::set_graphics_pipeline_buffer_srv(cmd, m_VisibilityGP, 1, targetMesh.indexedBisectorBuffer);

        // Draw the geometry
        d3d12::command_buffer::draw_procedural_indirect(cmd, m_VisibilityGP, targetMesh.indirectDrawBuffer);
    }
    d3d12::command_buffer::end_section(cmd);
}

void EarthRenderer::render_material(CommandBuffer cmd, RenderTexture colorBuffer, RenderTexture visibilityBuffer, RenderTexture depthTexture, const Planet& planet, Texture sgTexture, const Sky& sky, ConstantBuffer globalCB, ConstantBuffer deformationCB)
{
    // Get the mesh
    const CBTMesh& targetMesh = planet.get_cbt_mesh();

    // Get the size of the texture
    uint32_t width, height, depth;
    d3d12::graphics_resources::render_texture_dimensions(depthTexture, width, height, depth);

    // Render the mesh
    d3d12::command_buffer::start_section(cmd, "Render Earth Material");
    {
        // Constant buffers
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_MaterialCS, GLOBAL_CB_BINDING_SLOT, globalCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_MaterialCS, PLANET_CB_BINDING_SLOT, planet.get_planet_cb());
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_MaterialCS, SKY_ATMOSPHERE_BUFFER_SLOT, sky.constant_buffer());
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_MaterialCS, DEFORMATION_CB_BINDING_SLOT, deformationCB);

        // SRVs
        d3d12::command_buffer::set_compute_shader_buffer_srv(cmd, m_MaterialCS, 0, targetMesh.currentVertexBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_srv(cmd, m_MaterialCS, 1, targetMesh.currentDisplacementBuffer);
        d3d12::command_buffer::set_compute_shader_texture_srv(cmd, m_MaterialCS, 2, sky.transmittance_lut());
        d3d12::command_buffer::set_compute_shader_texture_srv(cmd, m_MaterialCS, 3, sky.multi_scattering_lut());
        d3d12::command_buffer::set_compute_shader_texture_srv(cmd, m_MaterialCS, 4, sgTexture);
        d3d12::command_buffer::set_compute_shader_render_texture_srv(cmd, m_MaterialCS, 5, visibilityBuffer);

        // UAVs
        d3d12::command_buffer::set_compute_shader_render_texture_uav(cmd, m_MaterialCS, 0, colorBuffer);

        // Sampler
        d3d12::command_buffer::set_compute_shader_sampler(cmd, m_MaterialCS, 0, m_LinearWrap);
        d3d12::command_buffer::set_compute_shader_sampler(cmd, m_MaterialCS, 1, m_LinearClamp);

        // Draw the geometry
        d3d12::command_buffer::dispatch(cmd, m_MaterialCS, (width + 7) / 8, (height + 7) / 8, 1);
        d3d12::command_buffer::uav_barrier_buffer(cmd, colorBuffer);

        // Restore the color and depth buffers
        d3d12::command_buffer::set_render_texture(cmd, colorBuffer, depthTexture);
    }

    d3d12::command_buffer::end_section(cmd);
}

void EarthRenderer::render_solid_wire(CommandBuffer cmd, RenderTexture colorBuffer, RenderTexture depthTexture, const Planet& planet, Texture sgTexture, const Sky& sky, ConstantBuffer globalCB, ConstantBuffer deformationCB)
{
    // Get the mesh
    const CBTMesh& targetMesh = planet.get_cbt_mesh();

    // Render the mesh
    d3d12::command_buffer::start_section(cmd, "Render Earth SolidWire");
    {
        // Constant buffers
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_SolidWireGP, GLOBAL_CB_BINDING_SLOT, globalCB);
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_SolidWireGP, PLANET_CB_BINDING_SLOT, planet.get_planet_cb());
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_SolidWireGP, SKY_ATMOSPHERE_BUFFER_SLOT, sky.constant_buffer());
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_SolidWireGP, DEFORMATION_CB_BINDING_SLOT, deformationCB);
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_SolidWireGP, UPDATE_CB_BINDING_SLOT, planet.get_update_cb());

        // SRVs
        d3d12::command_buffer::set_graphics_pipeline_buffer_srv(cmd, m_SolidWireGP, 0, targetMesh.currentVertexBuffer);
        d3d12::command_buffer::set_graphics_pipeline_buffer_srv(cmd, m_SolidWireGP, 1, targetMesh.currentDisplacementBuffer);
        d3d12::command_buffer::set_graphics_pipeline_buffer_srv(cmd, m_SolidWireGP, 2, targetMesh.indexedBisectorBuffer);
        d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmd, m_SolidWireGP, 3, sky.transmittance_lut());
        d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmd, m_SolidWireGP, 4, sky.multi_scattering_lut());
        d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmd, m_SolidWireGP, 5, sgTexture);

        // Sampler
        d3d12::command_buffer::set_graphics_pipeline_sampler(cmd, m_SolidWireGP, 0, m_LinearWrap);
        d3d12::command_buffer::set_graphics_pipeline_sampler(cmd, m_SolidWireGP, 1, m_LinearClamp);

        // Draw
        d3d12::command_buffer::draw_procedural_indirect(cmd, m_SolidWireGP, targetMesh.indirectDrawBuffer);
    }
    d3d12::command_buffer::end_section(cmd);
}

void EarthRenderer::render_planet_impostor(CommandBuffer cmd, const Planet& planet, const WaterData& waterData, const Sky& sky, ConstantBuffer globalCB)
{
    d3d12::command_buffer::start_section(cmd, "Render Earth (impostor)");
    {
        // Constant buffers
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_ImpostorGP, GLOBAL_CB_BINDING_SLOT, globalCB);
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_ImpostorGP, PLANET_CB_BINDING_SLOT, planet.get_planet_cb());
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_ImpostorGP, SKY_ATMOSPHERE_BUFFER_SLOT, sky.constant_buffer());
        d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmd, m_ImpostorGP, DEFORMATION_CB_BINDING_SLOT, waterData.get_deformation_cb());

        // SRV
        d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmd, m_ImpostorGP, 0, sky.transmittance_lut());
        d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmd, m_ImpostorGP, 1, sky.multi_scattering_lut());

        // Samplers
        d3d12::command_buffer::set_graphics_pipeline_sampler(cmd, m_ImpostorGP, 0, m_LinearClamp);

        // Dispatch
        d3d12::command_buffer::draw_procedural(cmd, m_ImpostorGP, 1, 1);
    }
    d3d12::command_buffer::end_section(cmd);
}
