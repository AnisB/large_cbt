// Project includes
#include "graphics/dx12_backend.h"

// Project includes
#include "mesh/mesh.h"

void initialize_cbt_mesh(const CPUMesh& cpuMesh, const CBT& cbt, GraphicsDevice device, CommandQueue queue, CommandBuffer cmdB, CBTMesh& cbtMesh)
{
    // Initialize the device cbt
    initialize_gpu_cbt(cbt, device, queue, cmdB, cbtMesh.gpuCBT);

    // Create the graphics buffer to upload, process and readback the bitfield buffer
    GraphicsBuffer heapIDUploadBuffer = 0;
    GraphicsBuffer neighborsUploadBuffer = 0;

    // We reset the command buffer
    d3d12::command_buffer::reset(cmdB);

    // Total number of bisector elements
    cbtMesh.totalNumElements = cpuMesh.totalNumElements;
    cbtMesh.numBaseVertices = (uint32_t)cpuMesh.basePoints.size();
    cbtMesh.baseDepth = cpuMesh.minimalDepth;

    // Bisector buffers
    cbtMesh.heapIDBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint64_t) * cpuMesh.totalNumElements, sizeof(uint64_t), GraphicsBufferType::Default);
    {
        heapIDUploadBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint64_t) * cpuMesh.totalNumElements, sizeof(uint64_t), GraphicsBufferType::Upload);
        d3d12::graphics_resources::set_buffer_data(heapIDUploadBuffer, (const char*)cpuMesh.heapIDArray.data(), sizeof(uint64_t) * cpuMesh.totalNumElements);
        d3d12::command_buffer::copy_graphics_buffer(cmdB, heapIDUploadBuffer, cbtMesh.heapIDBuffer);
    }

    cbtMesh.currentNeighborsBufferIdx = 0;
    cbtMesh.neighborsBuffers[0] = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint3) * cpuMesh.totalNumElements, sizeof(uint3), GraphicsBufferType::Default);
    {
        neighborsUploadBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint3) * cpuMesh.totalNumElements, sizeof(uint3), GraphicsBufferType::Upload);
        d3d12::graphics_resources::set_buffer_data(neighborsUploadBuffer, (const char*)cpuMesh.neighborsArray.data(), sizeof(uint3) * cpuMesh.totalNumElements);
        d3d12::command_buffer::copy_graphics_buffer(cmdB, neighborsUploadBuffer, cbtMesh.neighborsBuffers[0]);
    }
    cbtMesh.neighborsBuffers[1] = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint3) * cpuMesh.totalNumElements, sizeof(uint3), GraphicsBufferType::Default);

    // Intermediate buffers
    cbtMesh.updateBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(BisectorData) * cpuMesh.totalNumElements, sizeof(BisectorData), GraphicsBufferType::Default);
    cbtMesh.classificationBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint32_t) * (2 + cpuMesh.totalNumElements * 2), sizeof(uint32_t), GraphicsBufferType::Default);
    cbtMesh.simplificationBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint32_t) * (1 + cpuMesh.totalNumElements), sizeof(uint32_t), GraphicsBufferType::Default);
    cbtMesh.allocateBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint32_t) * (1 + cpuMesh.totalNumElements), sizeof(uint32_t), GraphicsBufferType::Default);
    cbtMesh.propagateBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint32_t) * (2 + cpuMesh.totalNumElements), sizeof(uint32_t), GraphicsBufferType::Default);
    
    // Active bisector indexation
    cbtMesh.indirectDrawBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint32_t) * (4 * 2 + 2), sizeof(uint32_t), GraphicsBufferType::Default);
    cbtMesh.indirectDispatchBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint32_t) * 3 * 3, sizeof(uint32_t), GraphicsBufferType::Default);
    cbtMesh.indexedBisectorBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint32_t) * cpuMesh.totalNumElements, sizeof(uint32_t), GraphicsBufferType::Default);
    cbtMesh.visibleIndexedBisectorBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint32_t) * cpuMesh.totalNumElements, sizeof(uint32_t), GraphicsBufferType::Default);
    cbtMesh.modifiedIndexedBisectorBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(uint32_t) * cpuMesh.totalNumElements, sizeof(uint32_t), GraphicsBufferType::Default);

    // Allocate the current vertex buffer
    if (d3d12::graphics_device::double_ops_support(device))
        cbtMesh.lebVertexBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(double3) * cbtMesh.totalNumElements * 4, sizeof(double3), GraphicsBufferType::Default);
    else
        cbtMesh.lebVertexBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(float3) * cbtMesh.totalNumElements * 4, sizeof(float3), GraphicsBufferType::Default);
    cbtMesh.currentVertexBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(float3) * cbtMesh.totalNumElements * 4, sizeof(float3), GraphicsBufferType::Default);
    cbtMesh.currentDisplacementBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(float3) * cbtMesh.totalNumElements * 3, sizeof(float3), GraphicsBufferType::Default);

    // Execute and flush the queue
    d3d12::command_buffer::close(cmdB);
    d3d12::command_queue::execute_command_buffer(queue, cmdB);
    d3d12::command_queue::flush(queue);

    // Make sure to free the temporary graphics buffers
    d3d12::graphics_resources::destroy_graphics_buffer(heapIDUploadBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(neighborsUploadBuffer);
}

void release_cbt_mesh(CBTMesh& cbtMesh)
{
    // Geometry buffers
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.currentDisplacementBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.currentVertexBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.lebVertexBuffer);

    // Indexation buffers
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.modifiedIndexedBisectorBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.visibleIndexedBisectorBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.indexedBisectorBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.indirectDispatchBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.indirectDrawBuffer);

    // Intermediate buffers
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.classificationBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.simplificationBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.allocateBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.updateBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.propagateBuffer);

    // Bisector buffers
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.heapIDBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.neighborsBuffers[0]);
    d3d12::graphics_resources::destroy_graphics_buffer(cbtMesh.neighborsBuffers[1]);

    // Release the cbt
    release_gpu_cbt(cbtMesh.gpuCBT);
}

void initialize_base_mesh(const CPUMesh& cpuMesh, GraphicsDevice device, CommandQueue queue, CommandBuffer buffer, BaseMesh& baseMesh)
{
    GraphicsBuffer vertexUploadBuffer = 0;
    GraphicsBuffer indexUploadBuffer = 0;

    // We reset the command buffer
    d3d12::command_buffer::reset(buffer);

    // Base vertex buffer
    uint32_t numBaseVertex = (uint32_t)cpuMesh.basePoints.size();
    baseMesh.numVertices = numBaseVertex;
    baseMesh.numElements = numBaseVertex / 3;
    uint32_t baseVertexBufferSize = sizeof(float3) * numBaseVertex;
    baseMesh.vertexBuffer = d3d12::graphics_resources::create_graphics_buffer(device, baseVertexBufferSize, sizeof(float3), GraphicsBufferType::Default);
    {
        vertexUploadBuffer = d3d12::graphics_resources::create_graphics_buffer(device, baseVertexBufferSize, sizeof(float3), GraphicsBufferType::Upload);
        d3d12::graphics_resources::set_buffer_data(vertexUploadBuffer, (const char*)cpuMesh.basePoints.data(), baseVertexBufferSize);
        d3d12::command_buffer::copy_graphics_buffer(buffer, vertexUploadBuffer, baseMesh.vertexBuffer);
    }

    // Index Buffer
    uint32_t indexBufferSize = cpuMesh.totalNumElements * sizeof(uint3);
    baseMesh.indexBuffer = d3d12::graphics_resources::create_graphics_buffer(device, indexBufferSize, sizeof(uint3), GraphicsBufferType::Default);
    {
        // Create the CPU version
        std::vector<uint3> indices(cpuMesh.totalNumElements);
        for (uint32_t eleIdx = 0; eleIdx < cpuMesh.totalNumElements; ++eleIdx)
            indices[eleIdx] = { 3 * eleIdx, 3 * eleIdx + 1, 3 * eleIdx + 2 };

        // Allocate the upload buffer
        indexUploadBuffer = d3d12::graphics_resources::create_graphics_buffer(device, indexBufferSize, sizeof(uint3), GraphicsBufferType::Upload);

        // Set the data
        d3d12::graphics_resources::set_buffer_data(indexUploadBuffer, (const char*)indices.data(), indexBufferSize);

        // Copy to the runtime buffer
        d3d12::command_buffer::copy_graphics_buffer(buffer, indexUploadBuffer, baseMesh.indexBuffer);
    }

    // Execute and flush the queue
    d3d12::command_buffer::close(buffer);
    d3d12::command_queue::execute_command_buffer(queue, buffer);
    d3d12::command_queue::flush(queue);

    // Make sure to free the temporary graphics buffers
    d3d12::graphics_resources::destroy_graphics_buffer(vertexUploadBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(indexUploadBuffer);
}

void release_base_mesh(BaseMesh& baseMesh)
{
    d3d12::graphics_resources::destroy_graphics_buffer(baseMesh.vertexBuffer);
    d3d12::graphics_resources::destroy_graphics_buffer(baseMesh.indexBuffer);
}
