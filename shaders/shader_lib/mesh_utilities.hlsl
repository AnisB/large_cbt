#ifndef MESH_UTILITIES_H
#define MESH_UTILITIES_H

// Define which variant we'll be using
#ifdef LEB_DOUBLE
#define LEB_DATA_TYPE double
#define LEB_DATA_TYPE3 double3
#else
#define LEB_DATA_TYPE float
#define LEB_DATA_TYPE3 float3
#endif

// include the leb library
#include "shader_lib/leb.hlsl"

// Needs to be defined before including mesh_utilities
struct Triangle
{
    LEB_DATA_TYPE3 p[3];
};

void EvaluateElementPosition(uint64_t heapID, uint32_t vertexDataOffset, uint minDepth, StructuredBuffer<float3> vertexBuffer, out Triangle parentTri, out Triangle childTri)
{
    // Get the depth of the element
    uint depth = HeapIDDepth(heapID);

    // Compute the required shift to find the original vertices
    uint64_t subTreeDepth = depth - minDepth;

    // Compute the base heapID
    uint64_t baseHeapID = 1u << (minDepth - 1);
    uint primitiveID = uint((heapID >> subTreeDepth) - baseHeapID);

    // Grab the base positions of the element
    LEB_DATA_TYPE3 p0 = LEB_DATA_TYPE3(vertexBuffer[3 * primitiveID + vertexDataOffset]);
    LEB_DATA_TYPE3 p1 = LEB_DATA_TYPE3(vertexBuffer[3 * primitiveID + 1 + vertexDataOffset]);
    LEB_DATA_TYPE3 p2 = LEB_DATA_TYPE3(vertexBuffer[3 * primitiveID + 2 + vertexDataOffset]);

    // Heap ID in the sub triangle
    uint64_t mask = subTreeDepth != 0uL ? 0xFFFFFFFFFFFFFFFFull >> (64ull - subTreeDepth) : 0ull;
    uint64_t baseHeap = (1ull << subTreeDepth);
    uint64_t baseMask = (mask & heapID);
    uint64_t subHeapID = baseMask + baseHeap;

    // Generate the triangle positions
    LEB_DATA_TYPE3 childArray[3] = {{p0.x, p1.x, p2.x}, {p0.y, p1.y, p2.y}, {p0.z, p1.z, p2.z}};
    LEB_DATA_TYPE3 parentArray[3];

    // Decode
    leb_DecodeNodeAttributeArray_parent_child(subHeapID, childArray, parentArray);

    // Fill the parent triangle
    parentTri.p[0] = LEB_DATA_TYPE3(parentArray[0][0], parentArray[1][0], parentArray[2][0]);
    parentTri.p[1] = LEB_DATA_TYPE3(parentArray[0][1], parentArray[1][1], parentArray[2][1]);
    parentTri.p[2] = LEB_DATA_TYPE3(parentArray[0][2], parentArray[1][2], parentArray[2][2]);

    // Fill the child triangle
    Triangle child;
    childTri.p[0] = LEB_DATA_TYPE3(childArray[0][0], childArray[1][0], childArray[2][0]);
    childTri.p[1] = LEB_DATA_TYPE3(childArray[0][1], childArray[1][1], childArray[2][1]);
    childTri.p[2] = LEB_DATA_TYPE3(childArray[0][2], childArray[1][2], childArray[2][2]);
}

#endif //MESH_UTILITIES_H
