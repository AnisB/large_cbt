#pragma once

// Project includes
#include "graphics/types.h"

// Project includes
#include "cbt/bisector.h"
#include "cbt/cbt.h"
#include "mesh/cpu_mesh.h"

struct BaseMesh
{
    // Number of vertices of the mesh
    uint32_t numVertices;
    // Vertex buffer
    GraphicsBuffer vertexBuffer;

    // Num primitives
    uint32_t numElements;
    // Index buffer
    GraphicsBuffer indexBuffer;
};

struct CBTMesh
{
    // Total number of elements
    uint32_t totalNumElements;

    // The base number of vertices
    uint32_t numBaseVertices;

    // Base heap depth
    uint32_t baseDepth;

    // Bisector buffers
    GraphicsBuffer heapIDBuffer;
    uint32_t currentNeighborsBufferIdx;
    GraphicsBuffer neighborsBuffers[2];

    // Intermediate buffers
    GraphicsBuffer updateBuffer;
    GraphicsBuffer classificationBuffer;
    GraphicsBuffer simplificationBuffer;
    GraphicsBuffer allocateBuffer;
    GraphicsBuffer propagateBuffer;

    // Indexation buffers
    GraphicsBuffer indirectDrawBuffer;
    GraphicsBuffer indirectDispatchBuffer;
    GraphicsBuffer indexedBisectorBuffer;
    GraphicsBuffer visibleIndexedBisectorBuffer;
    GraphicsBuffer modifiedIndexedBisectorBuffer;

    // Geometry buffers
    GraphicsBuffer lebVertexBuffer;
    GraphicsBuffer currentVertexBuffer;
    GraphicsBuffer currentDisplacementBuffer;

    // Device version of the CBT
    GPU_CBT gpuCBT;
};

// Function to initialize a cbt mesh
void initialize_cbt_mesh(const CPUMesh& cpuMesh, const CBT& cbt, GraphicsDevice device, CommandQueue queue, CommandBuffer buffer, CBTMesh& cbtMesh);
void release_cbt_mesh(CBTMesh& cbtMesh);

// Function to initialize a base mesh
void initialize_base_mesh(const CPUMesh& cpuMesh, GraphicsDevice device, CommandQueue queue, CommandBuffer buffer, BaseMesh& baseMesh);
void release_base_mesh(BaseMesh& baseMesh);
