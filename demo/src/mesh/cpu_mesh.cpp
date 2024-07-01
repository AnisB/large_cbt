// Project includes
#include "cbt/bisector.h"
#include "math/operators.h"
#include "mesh/cpu_mesh.h"
#include "tools/security.h"

// External includes
#define CC_IMPLEMENTATION
#include "ccmesh.h"

void fill_bisectors(cc_Mesh* targetMesh, uint32_t cbtNumElements,
    std::vector<uint64_t>& heapIDArray, std::vector<uint3>& neighborsArray,
    std::vector<float3>& basePoints, uint32_t& totalNumElements, uint32_t& minimalDepth)
{
    // Total number of elements of the mesh
    totalNumElements = targetMesh->halfedgeCount + cbtNumElements;

    // Allocate and initialize the memory
    heapIDArray.resize(totalNumElements);
    neighborsArray.resize(totalNumElements);
    memset(heapIDArray.data(), 0, totalNumElements * sizeof(uint64_t));
    memset(neighborsArray.data(), 0, totalNumElements * sizeof(uint3));

    // Find the base HeapID
    minimalDepth = std::min(find_msb_64(targetMesh->halfedgeCount), 63u) + 1;
    const uint64_t baseHeapID = 1ull << (minimalDepth - 1);

    // Set of points for each face
    basePoints.resize(targetMesh->halfedgeCount * 3);

    // Load the bisector data
    uint3 neighbors;
    for (uint32_t halfEdgeIdx = 0; halfEdgeIdx < (uint32_t)targetMesh->halfedgeCount; ++halfEdgeIdx)
    {
        // ID of this element
        uint32_t elementID = cbtNumElements + halfEdgeIdx;

        // Set the heap ID
        heapIDArray[elementID] = baseHeapID + halfEdgeIdx;

        // Fill and export the neighbors
        cc_Halfedge& halfEdge = targetMesh->halfedges[halfEdgeIdx];
        neighbors.x = halfEdge.prevID != -1 ? cbtNumElements + halfEdge.prevID : UINT32_MAX;
        neighbors.y = halfEdge.nextID != -1 ? cbtNumElements + halfEdge.nextID : UINT32_MAX;
        neighbors.z = halfEdge.twinID != -1 ? cbtNumElements + halfEdge.twinID : UINT32_MAX;
        neighborsArray[elementID] = neighbors;

        // First vertex
        cc_VertexPoint p2 = targetMesh->vertexPoints[halfEdge.vertexID];
        basePoints[3 * halfEdgeIdx + 2] = float3({ p2.x, p2.y, p2.z });

        // Second vertex
        cc_Halfedge& nextHalfEdge = targetMesh->halfedges[halfEdge.nextID];
        cc_VertexPoint p0 = targetMesh->vertexPoints[nextHalfEdge.vertexID];
        basePoints[3 * halfEdgeIdx + 0] = float3({ p0.x, p0.y, p0.z });

        // Let's compute the third vertex
        float3 thirdVertex = basePoints[3 * halfEdgeIdx + 2];
        float sumWeight = 1.0f;
        uint32_t currentIdx = halfEdge.nextID;
        while (currentIdx != halfEdgeIdx)
        {
            cc_Halfedge currentHalfEdge = targetMesh->halfedges[currentIdx];
            cc_VertexPoint vp = targetMesh->vertexPoints[currentHalfEdge.vertexID];
            thirdVertex.x += vp.x;
            thirdVertex.y += vp.y;
            thirdVertex.z += vp.z;
            sumWeight += 1.0f;
            currentIdx = currentHalfEdge.nextID;
        }
        basePoints[3 * halfEdgeIdx + 1] = float3({ thirdVertex.x / sumWeight, thirdVertex.y / sumWeight, thirdVertex.z / sumWeight });
    }
}

void load_cpu_mesh(const char* meshPath, uint32_t cbtNumElements, CPUMesh& outputMesh)
{
    // Load the CCMesh
    cc_Mesh* targetMesh = ccm_Load(meshPath);

    // Initialize the mesh
    fill_bisectors(targetMesh, cbtNumElements, outputMesh.heapIDArray, outputMesh.neighborsArray, outputMesh.basePoints, outputMesh.totalNumElements, outputMesh.minimalDepth);

    // Release the mesh
    ccm_Release(targetMesh);
}