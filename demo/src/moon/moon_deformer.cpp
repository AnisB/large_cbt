
// Project includes
#include "graphics/dx12_backend.h"
#include "moon/moon_deformer.h"
#include "render_pipeline/constant_buffers.h"
#include "render_pipeline/constants.h"
#include "tools/shader_utils.h"
#include "tools/texture_utils.h"
#include "tools/security.h"

// Cst & Dst
MoonDeformer::MoonDeformer()
{
}

MoonDeformer::~MoonDeformer()
{
}

void MoonDeformer::initialize(GraphicsDevice device, const char* textureLibrary)
{
    // Keep track of the device
    m_Device = device;

    // Sampelrs
    m_LinearWrap = { FilterMode::Linear, SamplerMode::Wrap, SamplerMode::Wrap, SamplerMode::Wrap };
}

void MoonDeformer::release()
{
    d3d12::compute_shader::destroy_compute_shader(m_EvaluateDeformationCS);
}

void MoonDeformer::reload_shaders(const std::string& shaderLibrary)
{
    // Deformation kernel
    ComputeShaderDescriptor csd;
    csd.includeDirectories.push_back(shaderLibrary);
    csd.filename = shaderLibrary + "\\Moon\\MoonDeformation.compute";
    csd.cbvCount = 4;
    csd.srvCount = 4;
    csd.uavCount = 3;
    csd.samplerCount = 1;
    csd.kernelname = "EvaluateDeformation";
    compile_and_replace_compute_shader(m_Device, csd, m_EvaluateDeformationCS);
}

void MoonDeformer::apply_deformation(CommandBuffer cmdB, const MoonMaterial& moonMat, ConstantBuffer globalCB, const CBTMesh& mesh, ConstantBuffer geometryCB, ConstantBuffer planetCB)
{
    d3d12::command_buffer::start_section(cmdB, "Moon Deformation");
    {
        // Constant buffers
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateDeformationCS, 0, globalCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateDeformationCS, 1, geometryCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateDeformationCS, 2, planetCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateDeformationCS, 3, moonMat.get_moon_cb());

        // SRVs
        d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_EvaluateDeformationCS, 0, moonMat.get_elevation_texture());
        d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_EvaluateDeformationCS, 1, moonMat.get_detail_texture());
        d3d12::command_buffer::set_compute_shader_buffer_srv(cmdB, m_EvaluateDeformationCS, 2, mesh.indexedBisectorBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_srv(cmdB, m_EvaluateDeformationCS, 3, mesh.indirectDrawBuffer);

        // UAVs
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmdB, m_EvaluateDeformationCS, 0, mesh.lebVertexBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmdB, m_EvaluateDeformationCS, 1, mesh.currentVertexBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmdB, m_EvaluateDeformationCS, 2, mesh.currentDisplacementBuffer);

        // Samplers
        d3d12::command_buffer::set_compute_shader_sampler(cmdB, m_EvaluateDeformationCS, 0, m_LinearWrap);

        // Dispatch
        d3d12::command_buffer::dispatch_indirect(cmdB, m_EvaluateDeformationCS, mesh.indirectDispatchBuffer, 3 * sizeof(uint32_t));

        // Barrier
        d3d12::command_buffer::uav_barrier_buffer(cmdB, mesh.currentVertexBuffer);
    }
    d3d12::command_buffer::end_section(cmdB);
}

