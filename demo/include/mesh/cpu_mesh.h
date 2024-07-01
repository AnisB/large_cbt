#pragma once

// Project includes
#include "math/types.h"

// System includes
#include <stdint.h>
#include <vector>

struct CPUMesh
{
    // Total number of elements
    uint32_t totalNumElements = 0;

    // Minimal depth of the mesh
    uint32_t minimalDepth = 0;

    // Bisector
    std::vector<uint64_t> heapIDArray;
    std::vector<uint3> neighborsArray;

    // Base positions
    std::vector<float3> basePoints;
};

// Load the CPU mesh
void load_cpu_mesh(const char* meshPath, uint32_t cbtNumElements, CPUMesh& outputMesh);