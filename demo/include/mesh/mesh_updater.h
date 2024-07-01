#pragma once

// Project includes
#include "cbt/cbt_utility.h"

// Project includes
#include "mesh/mesh.h"

// System includes
#include <string>

class MeshUpdater
{
public:
    // Cst & Dst
    MeshUpdater();
    ~MeshUpdater();

    // Allocate and release the resources
    void initialize(GraphicsDevice graphicsDevice, CommandQueue cmdQ);
    void release();

    // Reload shaders
    void reload_shaders(const std::string& shaderLibrary, CBTType cbtType, const char* updateShader = "UpdateMesh.compute");

    // Update a given mesh
    void update(CommandBuffer cmd, CBTMesh& mesh, ConstantBuffer globalCB, ConstantBuffer geometryCB, ConstantBuffer updateCB);

    // Make sure the mesh's topology is valid
    void validate(CommandBuffer cmd, const CBTMesh& mesh, ConstantBuffer geometryCB);

    // Reset the buffers
    void reset_buffers(CommandBuffer cmd, const CBTMesh& mesh);

    // Prepare the mesh for indirect dispatches
    void prepare_indirection(CommandBuffer cmd, const CBTMesh& mesh, ConstantBuffer geometryCB);

    // Control if the validation pass should run
    bool check_if_valid();

    // Query occupancy
    void query_occupancy(CommandBuffer cmdB, const CBTMesh& mesh);

    // Get the occupancy
    uint32_t get_occupancy();

private:
    // Graphics Device
    GraphicsDevice m_Device = 0;

    // Buffer that can be shared between the updates
    GraphicsBuffer indirectBuffer = 0;
    GraphicsBuffer memoryBuffer = 0;
    GraphicsBuffer validationBuffer = 0;
    GraphicsBuffer validationBufferRB = 0;
    GraphicsBuffer occupancyBufferRB = 0;

    // Main update
    ComputeShader m_ResetCS = 0;
    ComputeShader m_ClassifyCS = 0;
    ComputeShader m_SplitCS = 0;
    ComputeShader m_PrepareIndirectCS = 0;
    ComputeShader m_AllocateCS = 0;
    ComputeShader m_BisectCS = 0;
    ComputeShader m_PropagateBisectCS = 0;
    ComputeShader m_PrepareSimplifyCS = 0;
    ComputeShader m_SimplifyCS = 0;
    ComputeShader m_PropagateSimplifyCS = 0;

    // Reduction
    ComputeShader m_ReducePrePassCS = 0;
    ComputeShader m_ReduceFirstPassCS = 0;
    ComputeShader m_ReduceSecondPassCS = 0;

    // Indexation
    ComputeShader m_BisectorIndexationCS = 0;
    ComputeShader m_PrepareBisectorIndirectCS = 0;

    // Debug
    ComputeShader m_ValidateCS = 0;
};
