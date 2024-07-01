// Project includes
#include "water/water_deformer.h"

// Project includes
#include "graphics/dx12_backend.h"
#include "tools/security.h"
#include "tools/shader_utils.h"

// Binding slots for CBV
#define GLOBAL_CB_BINDING_SLOT 0
#define GEOMETRY_CB_BINDING_SLOT 1
#define PLANET_CB_BINDING_SLOT 2
#define UPDATE_CB_BINDING_SLOT 3
#define DEFORMATION_CB_BINDING_SLOT 4
#define WATER_DEFORMATION_CBV_COUNT 5

// Binding slots SRV
#define DISPLACEMENT_TEXTURE_BINDING_SLOT 0
#define INDEXED_BISECTOR_BUFFER_BINDING_SLOT 1
#define INDIRECT_DRAW_BUFFER_BINDING_SLOT 2
#define WATER_DEFORMATION_SRV_COUNT 3

// Binding slots UAV
#define LEB_VERTEX_BUFFER_SLOT 0
#define CURRENT_VERTEX_BUFFER_SLOT 1
#define CURRENT_DISPLACEMENT_BUFFER_SLOT 2
#define WATER_DEFORMATION_UAV_COUNT 3

// Samplers
#define DISPLACEMENT_SAMPLER_BINDING_SLOT 0
#define WATER_DEFORMATION_SAMPLER_COUNT 1

// Cst & Dst
WaterDeformer::WaterDeformer()
{
}

WaterDeformer::~WaterDeformer()
{
}

// Init and release
void WaterDeformer::initialize(GraphicsDevice device)
{
    // Keep track of the device
    m_Device = device;

    // Sampelrs
    m_LinearWrap = { FilterMode::Linear, SamplerMode::Wrap, SamplerMode::Wrap, SamplerMode::Wrap };
}

void WaterDeformer::release()
{
    d3d12::compute_shader::destroy_compute_shader(m_EvaluateDeformationCS);
}

void WaterDeformer::reload_shaders(const std::string& shaderLibrary)
{
    // Deformation kernel
    ComputeShaderDescriptor csd;
    csd.includeDirectories.push_back(shaderLibrary);
    csd.filename = shaderLibrary + "\\Water\\WaterDeformation.compute";
    csd.cbvCount = WATER_DEFORMATION_CBV_COUNT;
    csd.srvCount = WATER_DEFORMATION_SRV_COUNT;
    csd.uavCount = WATER_DEFORMATION_UAV_COUNT;
    csd.samplerCount = WATER_DEFORMATION_SAMPLER_COUNT;
    csd.kernelname = "EvaluateDeformation";
    compile_and_replace_compute_shader(m_Device, csd, m_EvaluateDeformationCS);
}

void WaterDeformer::apply_deformation(CommandBuffer cmdB, const CBTMesh& mesh, const WaterData& waterData, ConstantBuffer globalCB, ConstantBuffer geometryCB, ConstantBuffer planetCB, ConstantBuffer updateCB)
{
    d3d12::command_buffer::start_section(cmdB, "Water Deformation");
    {
        // CBVs
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateDeformationCS, GLOBAL_CB_BINDING_SLOT, globalCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateDeformationCS, GEOMETRY_CB_BINDING_SLOT, geometryCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateDeformationCS, PLANET_CB_BINDING_SLOT, planetCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateDeformationCS, UPDATE_CB_BINDING_SLOT, updateCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateDeformationCS, DEFORMATION_CB_BINDING_SLOT, waterData.get_deformation_cb());
        // SRVs
        d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_EvaluateDeformationCS, DISPLACEMENT_TEXTURE_BINDING_SLOT, waterData.get_displacement_texture());
        d3d12::command_buffer::set_compute_shader_buffer_srv(cmdB, m_EvaluateDeformationCS, INDEXED_BISECTOR_BUFFER_BINDING_SLOT, mesh.indexedBisectorBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_srv(cmdB, m_EvaluateDeformationCS, INDIRECT_DRAW_BUFFER_BINDING_SLOT, mesh.indirectDrawBuffer);
        // UAVs
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmdB, m_EvaluateDeformationCS, LEB_VERTEX_BUFFER_SLOT, mesh.lebVertexBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmdB, m_EvaluateDeformationCS, CURRENT_VERTEX_BUFFER_SLOT, mesh.currentVertexBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmdB, m_EvaluateDeformationCS, CURRENT_DISPLACEMENT_BUFFER_SLOT, mesh.currentDisplacementBuffer);
        // Samplers
        d3d12::command_buffer::set_compute_shader_sampler(cmdB, m_EvaluateDeformationCS, DISPLACEMENT_SAMPLER_BINDING_SLOT, m_LinearWrap);
        // Dispatch
        d3d12::command_buffer::dispatch_indirect(cmdB, m_EvaluateDeformationCS, mesh.indirectDispatchBuffer, 3 * sizeof(uint32_t));
        // Barrier
        d3d12::command_buffer::uav_barrier_buffer(cmdB, mesh.currentVertexBuffer);
    }
    d3d12::command_buffer::end_section(cmdB);
}

