// Project includes
#include "graphics/dx12_backend.h"
#include "imgui/imgui.h"
#include "math/operators.h"
#include "render_pipeline/constant_buffers.h"
#include "render_pipeline/planet.h"
#include "tools/shader_utils.h"
#include "tools/security.h"

#define WORKGROUP_SIZE 64

// CBVs
#define GLOBAL_CB_BINDING_SLOT CBV_SLOT(0)
#define GEOMETRY_CB_BINDING_SLOT CBV_SLOT(1)
#define PLANET_CB_BINDING_SLOT CBV_SLOT(2)
#define UPDATE_CB_BINDING_SLOT CBV_SLOT(3)

// SRVs
#define BASE_VERTEX_BUFFER_BINDING_SLOT SRV_SLOT(0)
#define HEAP_ID_BUFFER_BINDING_SLOT SRV_SLOT(1)
#define INDEXED_BISECTOR_BUFFER_BINDING_SLOT SRV_SLOT(2)
#define INDIRECT_DRAW_BUFFER_BINDING_SLOT SRV_SLOT(3)
#define LEB_MATRIX_CACHE_BINDING_SLOT SRV_SLOT(4)

// UAVs
#define CURRENT_VERTEX_BUFFER_SLOT UAV_SLOT(0)

// Counters
#define NUM_CBV_BINDING_SLOTS 4
#define NUM_SRV_BINDING_SLOTS 5
#define NUM_UAV_BINDING_SLOTS 1

Planet::Planet()
{
}

Planet::~Planet()
{
}

void Planet::initialize(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB,
                        float planetRadius, const double3& planetCenter, float toggleDistance, float triangleSize, uint32_t materialID,
                        const CPUMesh& mesh, const CBT& cbt)
    {
    // Keep track of the device
    m_Device = device;
    
    // Set the planet properties 
    m_PlanetRadius = planetRadius;
    m_PlanetCenter = planetCenter;
    m_ToggleDistance = toggleDistance;
    m_MaterialID = materialID;

    // Subdivision properties
    m_MaxSubdivisionDepth = 63;
    m_TriangleSize = triangleSize;

    // Allocate the required resources
    m_GeometryCB = d3d12::graphics_resources::create_constant_buffer(device, sizeof(GeometryCB), ConstantBufferType::Mixed);
    m_PlanetCB = d3d12::graphics_resources::create_constant_buffer(device, sizeof(PlanetCB), ConstantBufferType::Mixed);
    m_UpdateCB = d3d12::graphics_resources::create_constant_buffer(device, sizeof(UpdateCB), ConstantBufferType::Mixed);
    initialize_cbt_mesh(mesh, cbt, m_Device, cmdQ, cmdB, m_CBTMesh);
    initialize_base_mesh(mesh, m_Device, cmdQ, cmdB, m_BaseMesh);
}

void Planet::release()
{
    release_cbt_mesh(m_CBTMesh);
    release_base_mesh(m_BaseMesh);

    d3d12::graphics_resources::destroy_constant_buffer(m_UpdateCB);
    d3d12::graphics_resources::destroy_constant_buffer(m_PlanetCB);
    d3d12::graphics_resources::destroy_constant_buffer(m_GeometryCB);
    d3d12::compute_shader::destroy_compute_shader(m_LebEvalCS);
    d3d12::compute_shader::destroy_compute_shader(m_ClearCS);

    m_UpdateCB = 0;
    m_PlanetCB = 0;
    m_GeometryCB = 0;
    m_LebEvalCS = 0;
    m_ClearCS = 0;
}

void Planet::reload_shaders(const std::string& shaderLibrary)
{
    ComputeShaderDescriptor csd;
    csd.includeDirectories.push_back(shaderLibrary);
    csd.filename = shaderLibrary + "\\PlanetGeometry.compute";
    csd.cbvCount = 4;
    csd.srvCount = 5;
    csd.uavCount = 1;

    csd.kernelname = "EvaluateLEB";
    compile_and_replace_compute_shader(m_Device, csd, m_LebEvalCS);

    csd.kernelname = "ClearBuffer";
    compile_and_replace_compute_shader(m_Device, csd, m_ClearCS);
}

void Planet::render_ui(const char* planetName)
{
    ImGui::SliderInt((std::string("Max Depth ") + planetName).c_str(), &m_MaxSubdivisionDepth, m_CBTMesh.baseDepth, 63);
    ImGui::SliderFloat((std::string("Triangle Size ") + planetName).c_str(), &m_TriangleSize, 10.0f, 200.0f);
}

void Planet::upload_static_constant_buffers(CommandBuffer cmd)
{
    // This constant buffer only needs to be set once
    PlanetCB earthCB;
    earthCB._PlanetCenter = { (float)m_PlanetCenter.x, (float)m_PlanetCenter.y, (float)m_PlanetCenter.z };
    earthCB._PlanetRadius = (float)m_PlanetRadius;
    d3d12::graphics_resources::set_constant_buffer(m_PlanetCB, (const char*)&earthCB, sizeof(PlanetCB));
    d3d12::command_buffer::upload_constant_buffer(cmd, m_PlanetCB);

    // This constant buffer only needs to be set once
    GeometryCB geometryCB;
    geometryCB._TotalNumElements = m_CBTMesh.totalNumElements;
    geometryCB._TotalNumVertices = m_CBTMesh.totalNumElements * 3;
    geometryCB._BaseDepth = m_CBTMesh.baseDepth;
    geometryCB._MaterialID = m_MaterialID;
    d3d12::graphics_resources::set_constant_buffer(m_GeometryCB, (const char*)&geometryCB, sizeof(GeometryCB));
    d3d12::command_buffer::upload_constant_buffer(cmd, m_GeometryCB);
}

void Planet::update_constant_buffers(CommandBuffer cmd, const UpdateProperties& updateProperties)
{
    // Update constant buffer
    UpdateCB updateCB;

    // Camera update properties
    set_update_properties(updateProperties, updateCB);

    // Mesh properties
    updateCB._MaxSubdivisionDepth = m_MaxSubdivisionDepth;
    updateCB._TriangleSize = m_TriangleSize;
    
    // Set
    d3d12::graphics_resources::set_constant_buffer(m_UpdateCB, (const char*)&updateCB, sizeof(UpdateCB));

    // Upload
    d3d12::command_buffer::upload_constant_buffer(cmd, m_UpdateCB);
}

void Planet::evaluate_leb(CommandBuffer cmdB, const Camera& camera, ConstantBuffer globalCB, GraphicsBuffer lebMatrixCache, bool clear, bool complete)
{
    if (clear)
    {
        d3d12::command_buffer::start_section(cmdB, "Clear Buffer");
        {
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_ClearCS, 1, m_GeometryCB);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmdB, m_ClearCS, 0, m_CBTMesh.currentVertexBuffer);
            d3d12::command_buffer::dispatch(cmdB, m_ClearCS, (m_CBTMesh.totalNumElements * 3 + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE, 1, 1);
            d3d12::command_buffer::uav_barrier_buffer(cmdB, m_CBTMesh.currentVertexBuffer);
        }
        d3d12::command_buffer::end_section(cmdB);
    }

    d3d12::command_buffer::start_section(cmdB, "Evaluate Leb");
    {
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_LebEvalCS, 0, globalCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_LebEvalCS, 1, m_GeometryCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_LebEvalCS, 2, m_PlanetCB);
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_LebEvalCS, 3, m_UpdateCB);

        d3d12::command_buffer::set_compute_shader_buffer_srv(cmdB, m_LebEvalCS, 0, m_BaseMesh.vertexBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_srv(cmdB, m_LebEvalCS, 1, m_CBTMesh.heapIDBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_srv(cmdB, m_LebEvalCS, 2, complete ? m_CBTMesh.indexedBisectorBuffer : m_CBTMesh.modifiedIndexedBisectorBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_srv(cmdB, m_LebEvalCS, 3, m_CBTMesh.indirectDrawBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_srv(cmdB, m_LebEvalCS, 4, lebMatrixCache);

        d3d12::command_buffer::set_compute_shader_buffer_uav(cmdB, m_LebEvalCS, 0, m_CBTMesh.lebVertexBuffer);

        d3d12::command_buffer::dispatch_indirect(cmdB, m_LebEvalCS, m_CBTMesh.indirectDispatchBuffer, complete ? 0 : sizeof(uint32_t) * 6);
        d3d12::command_buffer::uav_barrier_buffer(cmdB, m_CBTMesh.currentVertexBuffer);
    }
    d3d12::command_buffer::end_section(cmdB);
}

void Planet::planet_visibility(const Camera& camera, double& distance, bool& visible, bool& updatable) const
{
    distance = length(m_PlanetCenter - camera.position);
    visible = distance < m_ToggleDistance;
    updatable = distance < m_ToggleDistance * 1.5;
}