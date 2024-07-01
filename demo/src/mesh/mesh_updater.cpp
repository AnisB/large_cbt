#pragma once

// Project includes
#include "graphics/dx12_backend.h"
#include "mesh/mesh_updater.h"
#include "tools/shader_utils.h"

// Main Update
const char* reset_kernel = "Reset";
const char* classify_kernel = "Classify";
const char* split_kernel = "Split";
const char* prepare_indirect_kernel = "PrepareIndirect";
const char* allocate_kernel = "Allocate";
const char* bisect_kernel = "Bisect";
const char* propagate_bisect_kernel = "PropagateBisect";
const char* prepare_simplify_kernel = "PrepareSimplify";
const char* simplify_kernel = "Simplify";
const char* propagate_simplify_kernel = "PropagateSimplify";

// Reduction
const char* reduce_pre_pass_kernel = "ReducePrePass";
const char* reduce_first_pass_kernel = "ReduceFirstPass";
const char* reduce_second_pass_kernel = "ReduceSecondPass";

// Indexation
const char* bisector_indexation_kernel = "BisectorIndexation";
const char* prepare_bisector_indirect_kernel = "PrepareBisectorIndirect";

// Debug
const char* validate_kernel = "Validate";

// Workgroup size
#define WORKGROUP_SIZE 64

// CBVs
#define GLOBAL_CB_BINDING_SLOT CBV_SLOT(0)
#define GEOMETRY_CB_BINDING_SLOT CBV_SLOT(1)
#define UPDATE_CB_BINDING_SLOT CBV_SLOT(2)

// SRVs
#define CURRENT_VERTEX_BUFFER_SLOT SRV_SLOT(0)
#define INDEXED_BISECTOR_BUFFER_BINDING_SLOT SRV_SLOT(1)

// UAVs
// CBT buffers
#define CBT_BUFFER0_BINDING_SLOT UAV_SLOT(0)
#define CBT_BUFFER1_BINDING_SLOT UAV_SLOT(1)

// Bisector buffers
#define HEAP_ID_BUFFER_BINDING_SLOT UAV_SLOT(2)
#define NEIGHBORS_BUFFER_BINDING_SLOT UAV_SLOT(3)
#define NEIGHBORS_OUTPUT_BUFFER_BINDING_SLOT UAV_SLOT(4)

// Intermediate buffers
#define BISECTOR_DATA_BUFFER_BINDING_SLOT UAV_SLOT(5)
#define CLASSIFICATION_BUFFER_BINDING_SLOT UAV_SLOT(6)
#define SIMPLIFICATION_BUFFER_BINDING_SLOT UAV_SLOT(7)
#define ALLOCATE_BUFFER_BINDING_SLOT UAV_SLOT(8)
#define PROPAGATE_BUFFER_BINDING_SLOT UAV_SLOT(9)
#define MEMORY_BUFFER_BINDING_SLOT UAV_SLOT(10)

// Indexation buffers
#define INDIRECT_DISPATCH_BUFFER_BINDING_SLOT UAV_SLOT(11)
#define INDIRECT_DRAW_BUFFER_BINDING_SLOT UAV_SLOT(12)
#define BISECTOR_INDICES_BINDING_SLOT UAV_SLOT(13)
#define VISIBLE_BISECTOR_INDICES_BINDING_SLOT UAV_SLOT(14)
#define MODIFIED_BISECTOR_INDICES_BINDING_SLOT UAV_SLOT(15)

// Debug
#define VALIDATION_BUFFER_BINDING_SLOT UAV_SLOT(16)

#define NUM_SRV_BINDING_SLOTS 2
#define NUM_CBV_BINDING_SLOTS 3
#define NUM_UAV_BINDING_SLOTS 17

MeshUpdater::MeshUpdater()
{
}

MeshUpdater::~MeshUpdater()
{
}

// Allocate and release the resources
void MeshUpdater::initialize(GraphicsDevice graphicsDevice, CommandQueue cmdQ)
{
    // Keep track of the graphics device
    m_Device = graphicsDevice;

    // Shared buffers
    indirectBuffer = d3d12::graphics_resources::create_graphics_buffer(m_Device, sizeof(uint32_t) * 9, sizeof(uint32_t), GraphicsBufferType::Default);
    memoryBuffer = d3d12::graphics_resources::create_graphics_buffer(m_Device, sizeof(int32_t) * 2, sizeof(int32_t), GraphicsBufferType::Default);
    validationBuffer = d3d12::graphics_resources::create_graphics_buffer(m_Device, sizeof(int32_t) * 2, sizeof(int32_t), GraphicsBufferType::Default);
    validationBufferRB = d3d12::graphics_resources::create_graphics_buffer(m_Device, sizeof(int32_t) * 2, sizeof(int32_t), GraphicsBufferType::Readback);
    occupancyBufferRB = d3d12::graphics_resources::create_graphics_buffer(m_Device, sizeof(uint32_t), sizeof(uint32_t), GraphicsBufferType::Readback);
}

void MeshUpdater::release()
{
    // Destroy the buffers
    d3d12::graphics_resources::destroy_graphics_buffer(occupancyBufferRB);
    d3d12::graphics_resources::destroy_graphics_buffer(validationBufferRB);
    d3d12::graphics_resources::destroy_graphics_buffer(validationBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(memoryBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(indirectBuffer);

    // Debug
    d3d12::compute_shader::destroy_compute_shader(m_ValidateCS);

    // Indexation
    d3d12::compute_shader::destroy_compute_shader(m_BisectorIndexationCS);
    d3d12::compute_shader::destroy_compute_shader(m_PrepareBisectorIndirectCS);

    // Reduce
    d3d12::compute_shader::destroy_compute_shader(m_ReducePrePassCS);
    d3d12::compute_shader::destroy_compute_shader(m_ReduceFirstPassCS);
    d3d12::compute_shader::destroy_compute_shader(m_ReduceSecondPassCS);

    // Main Update
    d3d12::compute_shader::destroy_compute_shader(m_PropagateSimplifyCS);
    d3d12::compute_shader::destroy_compute_shader(m_SimplifyCS);
    d3d12::compute_shader::destroy_compute_shader(m_PrepareSimplifyCS);
    d3d12::compute_shader::destroy_compute_shader(m_PropagateBisectCS);
    d3d12::compute_shader::destroy_compute_shader(m_BisectCS);
    d3d12::compute_shader::destroy_compute_shader(m_AllocateCS);
    d3d12::compute_shader::destroy_compute_shader(m_PrepareIndirectCS);
    d3d12::compute_shader::destroy_compute_shader(m_SplitCS);
    d3d12::compute_shader::destroy_compute_shader(m_ClassifyCS);
    d3d12::compute_shader::destroy_compute_shader(m_ResetCS);
}

void MeshUpdater::reload_shaders(const std::string& shaderLibrary, CBTType cbtType, const char* updateShader)
{
    // Create the compute shaders
    ComputeShaderDescriptor csd;
    csd.includeDirectories.push_back(shaderLibrary);
    csd.filename = shaderLibrary + "\\" + updateShader;
    csd.cbvCount = NUM_CBV_BINDING_SLOTS;
    csd.srvCount = NUM_SRV_BINDING_SLOTS;
    csd.uavCount = NUM_UAV_BINDING_SLOTS;
    csd.defines.push_back(cbt_type_to_string(cbtType));

    // Reset kernel
    csd.kernelname = reset_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_ResetCS);

    // Classify kernel
    csd.kernelname = classify_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_ClassifyCS);

    // Split kernel
    csd.kernelname = split_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_SplitCS);

    // Indirect kernel
    csd.kernelname = prepare_indirect_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_PrepareIndirectCS);

    // Allocate kernel
    csd.kernelname = allocate_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_AllocateCS);

    // Bisect kernel
    csd.kernelname = bisect_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_BisectCS);

    // Propagate split kernel
    csd.kernelname = propagate_bisect_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_PropagateBisectCS);

    // Prepare simplify kernel
    csd.kernelname = prepare_simplify_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_PrepareSimplifyCS);

    // Simplify kernel
    csd.kernelname = simplify_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_SimplifyCS);

    // Propagate simplify kernel
    csd.kernelname = propagate_simplify_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_PropagateSimplifyCS);

    // Reduce PrePass
    csd.kernelname = reduce_pre_pass_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_ReducePrePassCS);

    // Reduce Passes
    csd.kernelname = reduce_first_pass_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_ReduceFirstPassCS);

    csd.kernelname = reduce_second_pass_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_ReduceSecondPassCS);

    // Bisector indexation kernel
    csd.kernelname = bisector_indexation_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_BisectorIndexationCS);

    // Prepare bisector indirect kernel
    csd.kernelname = prepare_bisector_indirect_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_PrepareBisectorIndirectCS);

    // Validate kernel
    csd.kernelname = validate_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_ValidateCS);
}

void MeshUpdater::update(CommandBuffer cmd, CBTMesh& mesh, ConstantBuffer globalCB, ConstantBuffer geometryCB, ConstantBuffer updateCB)
{
    // Num groups that need to be dispatched
    uint32_t numGroups = (mesh.totalNumElements + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;

    // Grab the two bisectors
    uint32_t nextNeighborsBufferIdx = (mesh.currentNeighborsBufferIdx + 1) % 2;
    GraphicsBuffer currentNeighborsBuffer = mesh.neighborsBuffers[mesh.currentNeighborsBufferIdx];
    GraphicsBuffer nextNeighborsBuffer = mesh.neighborsBuffers[nextNeighborsBufferIdx];

    d3d12::command_buffer::start_section(cmd, "Update Mesh");
    {
        // Reset pass
        reset_buffers(cmd, mesh);

        // Classify Pass
        d3d12::command_buffer::start_section(cmd, "Classify");
        {
            // CBVs
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_ClassifyCS, GLOBAL_CB_BINDING_SLOT, globalCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_ClassifyCS, GEOMETRY_CB_BINDING_SLOT, geometryCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_ClassifyCS, UPDATE_CB_BINDING_SLOT, updateCB);

            // SRVs
            d3d12::command_buffer::set_compute_shader_buffer_srv(cmd, m_ClassifyCS, CURRENT_VERTEX_BUFFER_SLOT, mesh.currentVertexBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_srv(cmd, m_ClassifyCS, INDEXED_BISECTOR_BUFFER_BINDING_SLOT, mesh.indexedBisectorBuffer);

            // UAVs
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ClassifyCS, INDIRECT_DRAW_BUFFER_BINDING_SLOT, mesh.indirectDrawBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ClassifyCS, HEAP_ID_BUFFER_BINDING_SLOT, mesh.heapIDBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ClassifyCS, BISECTOR_DATA_BUFFER_BINDING_SLOT, mesh.updateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ClassifyCS, CLASSIFICATION_BUFFER_BINDING_SLOT, mesh.classificationBuffer);

            // Dispatch
            d3d12::command_buffer::dispatch_indirect(cmd, m_ClassifyCS, mesh.indirectDispatchBuffer);

            // Barrier
            d3d12::command_buffer::uav_barrier_buffer(cmd, mesh.updateBuffer);
        }
        d3d12::command_buffer::end_section(cmd);

        // Prepare Indirect Pass
        d3d12::command_buffer::start_section(cmd, "Prepare indirect split");
        {
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareIndirectCS, ALLOCATE_BUFFER_BINDING_SLOT, mesh.classificationBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareIndirectCS, INDIRECT_DISPATCH_BUFFER_BINDING_SLOT, indirectBuffer);
            d3d12::command_buffer::dispatch(cmd, m_PrepareIndirectCS, 2, 1, 1);
            d3d12::command_buffer::uav_barrier_buffer(cmd, indirectBuffer);
        }
        d3d12::command_buffer::end_section(cmd);

        // Split Pass
        d3d12::command_buffer::start_section(cmd, "Split");
        {
            // CBVs
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_SplitCS, GLOBAL_CB_BINDING_SLOT, globalCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_SplitCS, GEOMETRY_CB_BINDING_SLOT, geometryCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_SplitCS, UPDATE_CB_BINDING_SLOT, updateCB);

            // UAVs
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_SplitCS, CLASSIFICATION_BUFFER_BINDING_SLOT, mesh.classificationBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_SplitCS, HEAP_ID_BUFFER_BINDING_SLOT, mesh.heapIDBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_SplitCS, BISECTOR_DATA_BUFFER_BINDING_SLOT, mesh.updateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_SplitCS, NEIGHBORS_BUFFER_BINDING_SLOT, currentNeighborsBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_SplitCS, MEMORY_BUFFER_BINDING_SLOT, memoryBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_SplitCS, ALLOCATE_BUFFER_BINDING_SLOT, mesh.allocateBuffer);

            // Dispatch
            d3d12::command_buffer::dispatch_indirect(cmd, m_SplitCS, indirectBuffer);

            // Barrier
            d3d12::command_buffer::uav_barrier_buffer(cmd, mesh.updateBuffer);
        }
        d3d12::command_buffer::end_section(cmd);

        // Prepare Indirect Pass
        d3d12::command_buffer::start_section(cmd, "Prepare indirect allocate");
        {
            // UAVs
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareIndirectCS, ALLOCATE_BUFFER_BINDING_SLOT, mesh.allocateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareIndirectCS, INDIRECT_DISPATCH_BUFFER_BINDING_SLOT, indirectBuffer);

            // Dispatch
            d3d12::command_buffer::dispatch(cmd, m_PrepareIndirectCS, 1, 1, 1);

            // Barrier
            d3d12::command_buffer::uav_barrier_buffer(cmd, indirectBuffer);
        }
        d3d12::command_buffer::end_section(cmd);

        // Alocate Pass
        d3d12::command_buffer::start_section(cmd, "Allocate");
        {
            // CBVs
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_AllocateCS, GLOBAL_CB_BINDING_SLOT, globalCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_AllocateCS, GEOMETRY_CB_BINDING_SLOT, geometryCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_AllocateCS, UPDATE_CB_BINDING_SLOT, updateCB);

            // UAVs
            for (uint32_t bufferIdx = 0; bufferIdx < mesh.gpuCBT.bufferCount; ++bufferIdx)
                d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_AllocateCS, CBT_BUFFER0_BINDING_SLOT + bufferIdx, mesh.gpuCBT.bufferArray[bufferIdx]);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_AllocateCS, ALLOCATE_BUFFER_BINDING_SLOT, mesh.allocateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_AllocateCS, BISECTOR_DATA_BUFFER_BINDING_SLOT, mesh.updateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_AllocateCS, MEMORY_BUFFER_BINDING_SLOT, memoryBuffer);

            // Dispatch
            d3d12::command_buffer::dispatch_indirect(cmd, m_AllocateCS, indirectBuffer);

            // Barrier
            d3d12::command_buffer::uav_barrier_buffer(cmd, mesh.updateBuffer);
        }
        d3d12::command_buffer::end_section(cmd);

        // Copy Pass
        d3d12::command_buffer::copy_graphics_buffer(cmd, currentNeighborsBuffer, nextNeighborsBuffer);

        // Bisect Pass
        d3d12::command_buffer::start_section(cmd, "Bisect");
        {
            // CBVs
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_BisectCS, GLOBAL_CB_BINDING_SLOT, globalCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_BisectCS, GEOMETRY_CB_BINDING_SLOT, geometryCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_BisectCS, UPDATE_CB_BINDING_SLOT, updateCB);

            // UAVs
            for (uint32_t bufferIdx = 0; bufferIdx < mesh.gpuCBT.bufferCount; ++bufferIdx)
                d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectCS, CBT_BUFFER0_BINDING_SLOT + bufferIdx, mesh.gpuCBT.bufferArray[bufferIdx]);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectCS, ALLOCATE_BUFFER_BINDING_SLOT, mesh.allocateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectCS, HEAP_ID_BUFFER_BINDING_SLOT, mesh.heapIDBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectCS, BISECTOR_DATA_BUFFER_BINDING_SLOT, mesh.updateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectCS, NEIGHBORS_BUFFER_BINDING_SLOT, currentNeighborsBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectCS, NEIGHBORS_OUTPUT_BUFFER_BINDING_SLOT, nextNeighborsBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectCS, PROPAGATE_BUFFER_BINDING_SLOT, mesh.propagateBuffer);

            // Dispatch
            d3d12::command_buffer::dispatch_indirect(cmd, m_BisectCS, indirectBuffer);

            // Barrier
            d3d12::command_buffer::uav_barrier_buffer(cmd, nextNeighborsBuffer);
        }
        d3d12::command_buffer::end_section(cmd);

        // Prepare Indirect Pass
        d3d12::command_buffer::start_section(cmd, "Prepare indirect propagate bisect");
        {
            // UAVs
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareIndirectCS, ALLOCATE_BUFFER_BINDING_SLOT, mesh.propagateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareIndirectCS, INDIRECT_DISPATCH_BUFFER_BINDING_SLOT, indirectBuffer);

            // Dispatch
            d3d12::command_buffer::dispatch(cmd, m_PrepareIndirectCS, 1, 1, 1);

            // Barrier
            d3d12::command_buffer::uav_barrier_buffer(cmd, indirectBuffer);
        }
        d3d12::command_buffer::end_section(cmd);

        // Propagate Split Pass
        d3d12::command_buffer::start_section(cmd, "Propagate Bisect");
        {
            // CBVs
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_PropagateBisectCS, GLOBAL_CB_BINDING_SLOT, globalCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_PropagateBisectCS, GEOMETRY_CB_BINDING_SLOT, geometryCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_PropagateBisectCS, UPDATE_CB_BINDING_SLOT, updateCB);

            // UAVs
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PropagateBisectCS, PROPAGATE_BUFFER_BINDING_SLOT, mesh.propagateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PropagateBisectCS, BISECTOR_DATA_BUFFER_BINDING_SLOT, mesh.updateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PropagateBisectCS, NEIGHBORS_BUFFER_BINDING_SLOT, nextNeighborsBuffer);

            // Dispatch
            d3d12::command_buffer::dispatch_indirect(cmd, m_PropagateBisectCS, indirectBuffer);

            // Barrier
            d3d12::command_buffer::uav_barrier_buffer(cmd, mesh.updateBuffer);
        }
        d3d12::command_buffer::end_section(cmd);

        // Simplify Pass
        d3d12::command_buffer::start_section(cmd, "Prepare Simplify");
        {
            // CBVs
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_PrepareSimplifyCS, GLOBAL_CB_BINDING_SLOT, globalCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_PrepareSimplifyCS, GEOMETRY_CB_BINDING_SLOT, geometryCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_PrepareSimplifyCS, UPDATE_CB_BINDING_SLOT, updateCB);

            // UAVs
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareSimplifyCS, CLASSIFICATION_BUFFER_BINDING_SLOT, mesh.classificationBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareSimplifyCS, HEAP_ID_BUFFER_BINDING_SLOT, mesh.heapIDBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareSimplifyCS, BISECTOR_DATA_BUFFER_BINDING_SLOT, mesh.updateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareSimplifyCS, NEIGHBORS_BUFFER_BINDING_SLOT, nextNeighborsBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareSimplifyCS, SIMPLIFICATION_BUFFER_BINDING_SLOT, mesh.simplificationBuffer);

            // Dispatch
            d3d12::command_buffer::dispatch_indirect(cmd, m_PrepareSimplifyCS, indirectBuffer, 3 * sizeof(uint32_t));

            // Barrier
            d3d12::command_buffer::uav_barrier_buffer(cmd, mesh.simplificationBuffer);
        }
        d3d12::command_buffer::end_section(cmd);

        // Prepare Indirect Pass
        d3d12::command_buffer::start_section(cmd, "Prepare Indirect Simplify");
        {
            // UAVs
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareIndirectCS, ALLOCATE_BUFFER_BINDING_SLOT, mesh.simplificationBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareIndirectCS, INDIRECT_DISPATCH_BUFFER_BINDING_SLOT, indirectBuffer);

            // Dispatch
            d3d12::command_buffer::dispatch(cmd, m_PrepareIndirectCS, 1, 1, 1);

            // Barrier
            d3d12::command_buffer::uav_barrier_buffer(cmd, indirectBuffer);
        }
        d3d12::command_buffer::end_section(cmd);

        // Simplify Pass
        d3d12::command_buffer::start_section(cmd, "Simplify");
        {
            // CBVs
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_SimplifyCS, GLOBAL_CB_BINDING_SLOT, globalCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_SimplifyCS, GEOMETRY_CB_BINDING_SLOT, geometryCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_SimplifyCS, UPDATE_CB_BINDING_SLOT, updateCB);

            // UAVs
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_SimplifyCS, SIMPLIFICATION_BUFFER_BINDING_SLOT, mesh.simplificationBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_SimplifyCS, BISECTOR_DATA_BUFFER_BINDING_SLOT, mesh.updateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_SimplifyCS, NEIGHBORS_BUFFER_BINDING_SLOT, nextNeighborsBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_SimplifyCS, HEAP_ID_BUFFER_BINDING_SLOT, mesh.heapIDBuffer);
            for (uint32_t bufferIdx = 0; bufferIdx < mesh.gpuCBT.bufferCount; ++bufferIdx)
                d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_SimplifyCS, CBT_BUFFER0_BINDING_SLOT + bufferIdx, mesh.gpuCBT.bufferArray[bufferIdx]);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_SimplifyCS, PROPAGATE_BUFFER_BINDING_SLOT, mesh.propagateBuffer);

            // Dispatch
            d3d12::command_buffer::dispatch_indirect(cmd, m_SimplifyCS, indirectBuffer);

            // Barrier
            d3d12::command_buffer::uav_barrier_buffer(cmd, nextNeighborsBuffer);
        }
        d3d12::command_buffer::end_section(cmd);

        // Prepare Indirect Pass
        d3d12::command_buffer::start_section(cmd, "Prepare indirect propagate simplify");
        {
            // UAVs
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareIndirectCS, ALLOCATE_BUFFER_BINDING_SLOT, mesh.propagateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareIndirectCS, INDIRECT_DISPATCH_BUFFER_BINDING_SLOT, indirectBuffer);

            // Dispatch
            d3d12::command_buffer::dispatch(cmd, m_PrepareIndirectCS, 2, 1, 1);

            // Barrier
            d3d12::command_buffer::uav_barrier_buffer(cmd, indirectBuffer);
        }
        d3d12::command_buffer::end_section(cmd);

        // Propagate Simplify Pass
        d3d12::command_buffer::start_section(cmd, "PropagateSimplify");
        {
            // CBVs
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_PropagateSimplifyCS, GLOBAL_CB_BINDING_SLOT, globalCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_PropagateSimplifyCS, GEOMETRY_CB_BINDING_SLOT, geometryCB);
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_PropagateSimplifyCS, UPDATE_CB_BINDING_SLOT, updateCB);

            // UAVs
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PropagateSimplifyCS, PROPAGATE_BUFFER_BINDING_SLOT, mesh.propagateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PropagateSimplifyCS, HEAP_ID_BUFFER_BINDING_SLOT, mesh.heapIDBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PropagateSimplifyCS, BISECTOR_DATA_BUFFER_BINDING_SLOT, mesh.updateBuffer);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PropagateSimplifyCS, NEIGHBORS_BUFFER_BINDING_SLOT, nextNeighborsBuffer);

            // Dispatch
            d3d12::command_buffer::dispatch_indirect(cmd, m_PropagateSimplifyCS, indirectBuffer, 3 * sizeof(uint32_t));

            // Barrier
            d3d12::command_buffer::uav_barrier_buffer(cmd, nextNeighborsBuffer);
        }
        d3d12::command_buffer::end_section(cmd);

        // Update Tree
        d3d12::command_buffer::start_section(cmd, "Reduce");
        {
            // The unique CBT Buffer
            GraphicsBuffer cbtBuffer0 = mesh.gpuCBT.bufferArray[0];
            GraphicsBuffer cbtBuffer1 = mesh.gpuCBT.bufferArray[1];

            // First Pass
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ReducePrePassCS, CBT_BUFFER0_BINDING_SLOT, cbtBuffer0);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ReducePrePassCS, CBT_BUFFER1_BINDING_SLOT, cbtBuffer1);
            d3d12::command_buffer::dispatch(cmd, m_ReducePrePassCS, mesh.gpuCBT.lastLevelSize / (4 * WORKGROUP_SIZE), 1, 1);
            d3d12::command_buffer::uav_barrier_buffer(cmd, cbtBuffer0);

            // Second Pass
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ReduceFirstPassCS, CBT_BUFFER0_BINDING_SLOT, cbtBuffer0);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ReduceFirstPassCS, CBT_BUFFER1_BINDING_SLOT, cbtBuffer1);
            d3d12::command_buffer::dispatch(cmd, m_ReduceFirstPassCS, 8, 1, 1);
            d3d12::command_buffer::uav_barrier_buffer(cmd, cbtBuffer0);

            // Third Pass
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ReduceSecondPassCS, CBT_BUFFER0_BINDING_SLOT, cbtBuffer0);
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ReduceSecondPassCS, CBT_BUFFER1_BINDING_SLOT, cbtBuffer1);
            d3d12::command_buffer::dispatch(cmd, m_ReduceSecondPassCS, 1, 1, 1);
            d3d12::command_buffer::uav_barrier_buffer(cmd, cbtBuffer0);

        }
        d3d12::command_buffer::end_section(cmd);

        // Move to the next bisector
        mesh.currentNeighborsBufferIdx = nextNeighborsBufferIdx;

        // Prepare for indirect dispatches
        prepare_indirection(cmd, mesh, geometryCB);
    }
    d3d12::command_buffer::end_section(cmd);
}

void MeshUpdater::validate(CommandBuffer cmd, const CBTMesh& mesh, ConstantBuffer geometryCB)
{
    d3d12::command_buffer::start_section(cmd, "Validate Mesh");
    {
        // Num groups that need to be dispatched
        uint32_t numGroups = (mesh.totalNumElements + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;

        // CBVs
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_ValidateCS, GEOMETRY_CB_BINDING_SLOT, geometryCB);

        // UAVs
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ValidateCS, HEAP_ID_BUFFER_BINDING_SLOT, mesh.heapIDBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ValidateCS, BISECTOR_DATA_BUFFER_BINDING_SLOT, mesh.updateBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ValidateCS, NEIGHBORS_BUFFER_BINDING_SLOT, mesh.neighborsBuffers[mesh.currentNeighborsBufferIdx]);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ValidateCS, VALIDATION_BUFFER_BINDING_SLOT, validationBuffer);

        // Dispatch
        d3d12::command_buffer::dispatch(cmd, m_ValidateCS, numGroups, 1, 1);

        // Barrier
        d3d12::command_buffer::uav_barrier_buffer(cmd, validationBuffer);

        // Copy
        d3d12::command_buffer::copy_graphics_buffer(cmd, validationBuffer, validationBufferRB);
    }
    d3d12::command_buffer::end_section(cmd);
}

void MeshUpdater::reset_buffers(CommandBuffer cmd, const CBTMesh& mesh)
{
    d3d12::command_buffer::start_section(cmd, "Reset Buffers");
    // Reset Pass
    {
        // UAVs
        for (uint32_t bufferIdx = 0; bufferIdx < mesh.gpuCBT.bufferCount; ++bufferIdx)
            d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ResetCS, CBT_BUFFER0_BINDING_SLOT + bufferIdx, mesh.gpuCBT.bufferArray[bufferIdx]);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ResetCS, MEMORY_BUFFER_BINDING_SLOT, memoryBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ResetCS, CLASSIFICATION_BUFFER_BINDING_SLOT, mesh.classificationBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ResetCS, ALLOCATE_BUFFER_BINDING_SLOT, mesh.allocateBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ResetCS, INDIRECT_DRAW_BUFFER_BINDING_SLOT, mesh.indirectDrawBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ResetCS, SIMPLIFICATION_BUFFER_BINDING_SLOT, mesh.simplificationBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_ResetCS, PROPAGATE_BUFFER_BINDING_SLOT, mesh.propagateBuffer);

        // Dispatch
        d3d12::command_buffer::dispatch(cmd, m_ResetCS, 1, 1, 1);

        // Barrier
        d3d12::command_buffer::uav_barrier_buffer(cmd, memoryBuffer);
    }
    d3d12::command_buffer::end_section(cmd);
}

void MeshUpdater::prepare_indirection(CommandBuffer cmd, const CBTMesh& mesh, ConstantBuffer geometryCB)
{
    // Num groups that need to be dispatched
    uint32_t numGroups = (mesh.totalNumElements + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;

    d3d12::command_buffer::start_section(cmd, "Prepare Indirect Draw & Dispatch");
    // Bisector indexation
    {
        // CBVs
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_BisectorIndexationCS, GEOMETRY_CB_BINDING_SLOT, geometryCB);

        // UAVs
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectorIndexationCS, HEAP_ID_BUFFER_BINDING_SLOT, mesh.heapIDBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectorIndexationCS, INDIRECT_DRAW_BUFFER_BINDING_SLOT, mesh.indirectDrawBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectorIndexationCS, BISECTOR_INDICES_BINDING_SLOT, mesh.indexedBisectorBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectorIndexationCS, BISECTOR_DATA_BUFFER_BINDING_SLOT, mesh.updateBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectorIndexationCS, NEIGHBORS_BUFFER_BINDING_SLOT, mesh.neighborsBuffers[mesh.currentNeighborsBufferIdx]);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectorIndexationCS, VISIBLE_BISECTOR_INDICES_BINDING_SLOT, mesh.visibleIndexedBisectorBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_BisectorIndexationCS, MODIFIED_BISECTOR_INDICES_BINDING_SLOT, mesh.modifiedIndexedBisectorBuffer);

        // Dispatch
        d3d12::command_buffer::dispatch(cmd, m_BisectorIndexationCS, numGroups, 1, 1);
    }

    // Barrier for both passes
    d3d12::command_buffer::uav_barrier_buffer(cmd, mesh.indirectDrawBuffer);

    // Prepare bisector indirect dispatch
    {
        // UAVs
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareBisectorIndirectCS, INDIRECT_DRAW_BUFFER_BINDING_SLOT, mesh.indirectDrawBuffer);
        d3d12::command_buffer::set_compute_shader_buffer_uav(cmd, m_PrepareBisectorIndirectCS, INDIRECT_DISPATCH_BUFFER_BINDING_SLOT, mesh.indirectDispatchBuffer);

        // Dispatch
        d3d12::command_buffer::dispatch(cmd, m_PrepareBisectorIndirectCS, 1, 1, 1);

        // Barrier
        d3d12::command_buffer::uav_barrier_buffer(cmd, mesh.indirectDispatchBuffer);
    }
    d3d12::command_buffer::end_section(cmd);
}

bool MeshUpdater::check_if_valid()
{
    uint32_t* buffer = (uint32_t*)d3d12::graphics_resources::allocate_cpu_buffer(validationBufferRB);
    bool valid = buffer[0] == 0;
    d3d12::graphics_resources::release_cpu_buffer(validationBufferRB);
    return valid;
}

// Query occupancy
void MeshUpdater::query_occupancy(CommandBuffer cmdB, const CBTMesh& mesh)
{
    d3d12::command_buffer::copy_graphics_buffer(cmdB, mesh.gpuCBT.bufferArray[0], 0, occupancyBufferRB, 0, sizeof(uint32_t));
}

// Get the occupancy
uint32_t MeshUpdater::get_occupancy()
{
    uint32_t* buffer = (uint32_t*)d3d12::graphics_resources::allocate_cpu_buffer(occupancyBufferRB);
    uint32_t occupancy = buffer[0];
    d3d12::graphics_resources::release_cpu_buffer(occupancyBufferRB);
    return occupancy;
}