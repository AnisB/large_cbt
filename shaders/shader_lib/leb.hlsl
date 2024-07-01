#ifndef LEB_D_H
#define LEB_D_H

#if defined(LEB_MATRIX_CACHE_BINDING_SLOT)
    // Resolution of the cache
    #define LEB_TABLE_DEPTH 5ULL

    // Cache buffer
    StructuredBuffer<float3x3> _LebMatrixCache: register(LEB_MATRIX_CACHE_BINDING_SLOT);

    // Cache in shared memory
    groupshared float3x3 g_MatrixCache[2ULL << LEB_TABLE_DEPTH];

    void load_leb_matrix_cache_to_shared_memory(uint groupIndex)
    {
        if (groupIndex < (2ULL << LEB_TABLE_DEPTH))
                g_MatrixCache[groupIndex] = _LebMatrixCache[groupIndex];
        GroupMemoryBarrierWithGroupSync();
    }
#endif

#if defined(UNSUPPORTED_FIRST_BIT_HIGH)
uint leb_depth(uint64_t heapID)
{
    uint depth = 0;
    while (heapID > 0u)
    {
        ++depth;
        heapID >>= 1u;
    }
    return depth - 1;
}
#else
uint leb_depth(uint64_t heapID)
{
    return heapID != 0 ? firstbithigh(heapID) : 0;
}
#endif

/*******************************************************************************
 * GetBitValue -- Returns the value of a bit stored in a 64-bit word
 *
 */

uint64_t leb__GetBitValue(uint64_t bitField, int64_t bitID)
{
    return ((bitField >> bitID) & 1L);
}

/*******************************************************************************
 * IdentityMatrix3x3 -- Sets a 3x3 matrix to identity
 *
 */

void leb__IdentityMatrix3x3(out float3x3 m)
{
    m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
    m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
    m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;
}

#if !defined(FP64_UNSUPPORTED)
void leb__IdentityMatrix3x3(out double3x3 m)
{
    m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0;
    m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0;
    m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0;
}
#endif

/*******************************************************************************
 * SplittingMatrix -- Computes a LEB splitting matrix from a split bit
 *
 */

void leb__SplittingMatrix(inout float3x3 mat, uint64_t bitValue)
{
    float b = (float)bitValue;
    float c = 1.0f - b;
    const float3x3 splitMatrix = {
        {0.0f, b, c},
        {0.5f, 0.0f, 0.5f},
        {b, c, 0.0f}
    };
    mat = mul(splitMatrix, mat);
}

#if !defined(FP64_UNSUPPORTED)
void leb__SplittingMatrix(inout double3x3 mat, uint64_t bitValue)
{
    double b = (double)bitValue;
    double c = 1.0 - b;
    const double3x3 splitMatrix = {
        {0.0, b, c},
        {0.5, 0.0, 0.5},
        {b, c, 0.0}
    };
    mat = mul(splitMatrix, mat);
}
#endif

/*******************************************************************************
 * SplittingMatrix -- Computes a LEB splitting matrix from a split bit
 *
 */

float3x3 leb__SplittingMatrix_out(float3x3 mat, uint64_t bitValue)
{
    float b = (float)bitValue;
    float c = 1.0 - b;
    float3x3 splitMatrix = {
        {0.0, b, c},
        {0.5, 0.0, 0.5},
        {b, c, 0.0}
    };
    return mul(splitMatrix, mat);
}

#if !defined(FP64_UNSUPPORTED)
double3x3 leb__SplittingMatrix_out(double3x3 mat, uint64_t bitValue)
{
    double b = (double)bitValue;
    double c = 1.0 - b;
    double3x3 splitMatrix = {
        {0.0, b, c},
        {0.5, 0.0, 0.5},
        {b, c, 0.0}
    };
    return mul(splitMatrix, mat);
}
#endif

/*******************************************************************************
 * DecodeTransformationMatrix -- Computes the matrix associated to a LEB
 * node
 *
*/

void leb__DecodeTransformationMatrix(uint64_t heapID, out float3x3 mat)
{
    int depth = leb_depth(heapID);
    leb__IdentityMatrix3x3(mat);
    for (int bitID = depth - 1; bitID >= 0; --bitID)
        leb__SplittingMatrix(mat, leb__GetBitValue(heapID, bitID));
}

#if !defined(FP64_UNSUPPORTED)
void leb__DecodeTransformationMatrix(uint64_t heapID, out double3x3 mat)
{
    int depth = leb_depth(heapID);
    leb__IdentityMatrix3x3(mat);
    for (int bitID = depth - 1; bitID >= 0; --bitID)
        leb__SplittingMatrix(mat, leb__GetBitValue(heapID, bitID));
}
#endif

#if defined(LEB_MATRIX_CACHE_BINDING_SLOT)
void leb__DecodeTransformationMatrix_Tabulated(uint64_t heapID, out float3x3 mat)
{
    leb__IdentityMatrix3x3(mat);
    const uint64_t msb = (1ULL << LEB_TABLE_DEPTH);
    const uint64_t mask = ~(~0ULL << LEB_TABLE_DEPTH);
    while (heapID > mask)
    {
        uint32_t index = uint32_t((heapID & mask) | msb);
        mat = mul(mat, g_MatrixCache[index]);
        heapID >>= LEB_TABLE_DEPTH;
    }
    mat = mul(mat, g_MatrixCache[uint32_t(heapID)]);
}

#if !defined(FP64_UNSUPPORTED)
void leb__DecodeTransformationMatrix_Tabulated(uint64_t heapID, out double3x3 mat)
{   
    uint64_t parentHeapID = heapID / 2ULL;
    const uint32_t msb = (1u << LEB_TABLE_DEPTH);
    const uint32_t mask = ~(~0u << LEB_TABLE_DEPTH);
    float3x3 m1, m2;
    leb__IdentityMatrix3x3(m1);
    leb__IdentityMatrix3x3(m2);

    // first part
    while ((heapID > mask) && (heapID > 0x00000000ffffffff))
    {
        uint32_t index = uint32_t((heapID & mask) | msb);
        m1 = mul(m1, g_MatrixCache[index]);
        heapID >>= LEB_TABLE_DEPTH;
    }

    // second part
    while (heapID > mask)
    {
        uint32_t index = uint32_t((heapID & mask) | msb);
        m2 = mul(m2, g_MatrixCache[index]);
        heapID >>= LEB_TABLE_DEPTH;
    }
    m2 = mul(m2, g_MatrixCache[heapID]);
    mat = mul((double3x3)m1, (double3x3)m2);
}
#endif
#endif

/*******************************************************************************
 * DecodeTransformationMatrix -- Computes the matrix associated to a LEB
 * node
 *
*/

void leb__DecodeTransformationMatrix_parent_child(uint64_t heapID, out float3x3 parent, out float3x3 child)
{
    int depth = leb_depth(heapID);
    leb__IdentityMatrix3x3(parent);

    // Evaluate the parent matrix
    int bitID;
    for (bitID = depth - 1; bitID > 0; --bitID)
        leb__SplittingMatrix(parent, leb__GetBitValue(heapID, bitID));

    // Evaluate the child
    if (depth > 0)
        child = leb__SplittingMatrix_out(parent, leb__GetBitValue(heapID, bitID));
    else
        child = parent;
}

#if !defined(FP64_UNSUPPORTED)
void leb__DecodeTransformationMatrix_parent_child(uint64_t heapID, out double3x3 parent, out double3x3 child)
{
    int depth = leb_depth(heapID);
    leb__IdentityMatrix3x3(parent);

    // Evaluate the parent matrix
    int bitID;
    for (bitID = depth - 1; bitID > 0; --bitID)
        leb__SplittingMatrix(parent, leb__GetBitValue(heapID, bitID));

    // Evaluate the child 
    child = leb__SplittingMatrix_out(parent, leb__GetBitValue(heapID, bitID));
}

void leb__DecodeTransformationMatrix_parent_child_optimized(uint64_t heapID, out double3x3 parent, out double3x3 child)
{
    int depth = leb_depth(heapID);
    float3x3 p0;
    leb__IdentityMatrix3x3(p0);

    // Evaluate the parent matrix
    int firstStep = max(depth - 1 - 32, 0);
    int bitID;
    for (bitID = depth - 1; bitID > firstStep; --bitID)
        leb__SplittingMatrix(p0, leb__GetBitValue(heapID, bitID));

    float3x3 p1;
    leb__IdentityMatrix3x3(p1);
    for (bitID = firstStep; bitID > 0; --bitID)
        leb__SplittingMatrix(p1, leb__GetBitValue(heapID, bitID));

    // Convert both matrices to double for the multiplcation
    double3x3 p0_d = (double3x3)p0;
    double3x3 p1_d = (double3x3)p1;
    parent = mul(p1_d, p0_d);

    // Evaluate the child
    if (depth > 0)
        child = leb__SplittingMatrix_out(parent, leb__GetBitValue(heapID, 0));
    else
        child = parent;
}
#endif

#if defined(LEB_MATRIX_CACHE_BINDING_SLOT)
void leb__DecodeTransformationMatrix_parent_child_Tabulated(uint64_t heapID, out float3x3 parent, out float3x3 child)
{
    int depth = leb_depth(heapID);
    leb__IdentityMatrix3x3(parent);
    const uint64_t msb = (1ULL << LEB_TABLE_DEPTH);
    const uint64_t mask = ~(~0ULL << LEB_TABLE_DEPTH);
    uint64_t parentHeapID = heapID / 2;
    while (parentHeapID > mask)
    {
        uint32_t index = uint32_t((parentHeapID & mask) | msb);
        parent = mul(parent, g_MatrixCache[index]);
        parentHeapID >>= LEB_TABLE_DEPTH;
    }
    if (parentHeapID != 0)
        parent = mul(parent, g_MatrixCache[uint32_t(parentHeapID)]);

    // Evaluate the child
    if (depth > 0)
        child = leb__SplittingMatrix_out(parent, leb__GetBitValue(heapID, 0));
    else
        child = parent;
}

#if !defined(FP64_UNSUPPORTED)
void leb__DecodeTransformationMatrix_parent_child_Tabulated(uint64_t heapID, out double3x3 parent, out double3x3 child)
{
    int depth = leb_depth(heapID);
    uint64_t parentHeapID = heapID / 2ULL;
    const uint32_t msb = (1u << LEB_TABLE_DEPTH);
    const uint32_t mask = ~(~0u << LEB_TABLE_DEPTH);
    float3x3 m1, m2;
    leb__IdentityMatrix3x3(m1);
    leb__IdentityMatrix3x3(m2);
    // first part
    while ((parentHeapID > mask) && (parentHeapID > 0x00000000ffffffff))
    {
        uint32_t index = uint32_t((parentHeapID & mask) | msb);
        m1 = mul(m1, g_MatrixCache[index]);
        parentHeapID >>= LEB_TABLE_DEPTH;
    }

    // second part
    while (parentHeapID > mask)
    {
        uint32_t index = uint32_t((parentHeapID & mask) | msb);
        m2 = mul(m2, g_MatrixCache[index]);
        parentHeapID >>= LEB_TABLE_DEPTH;
    }
    if (parentHeapID != 0)
        m2 = mul(m2, g_MatrixCache[parentHeapID]);
    parent = mul((double3x3)m1, (double3x3)m2);

    // Evaluate the child
    if (depth > 0)
        child = leb__SplittingMatrix_out(parent, leb__GetBitValue(heapID, 0));
    else
        child = parent;
}
#endif
#endif

/*******************************************************************************
 * DecodeNodeAttributeArray -- Compute the triangle attributes at the input node
 *
 */

void leb_DecodeNodeAttributeArray(uint64_t heapID, inout float3 attributeArray[2])
{
    float3x3 m;
    leb__DecodeTransformationMatrix(heapID, m);
    for (int i = 0; i < 2; ++i)
    {
        float3 attributeVector = attributeArray[i];
        attributeArray[i][0] = dot(m[0], attributeVector);
        attributeArray[i][1] = dot(m[1], attributeVector);
        attributeArray[i][2] = dot(m[2], attributeVector);
    }
}

#if !defined(FP64_UNSUPPORTED)
void leb_DecodeNodeAttributeArray(uint64_t heapID, inout double3 attributeArray[2])
{
    double3x3 m;
    leb__DecodeTransformationMatrix(heapID, m);
    for (int i = 0; i < 2; ++i)
    {
        double3 attributeVector = attributeArray[i];
        attributeArray[i][0] = dot(m[0], attributeVector);
        attributeArray[i][1] = dot(m[1], attributeVector);
        attributeArray[i][2] = dot(m[2], attributeVector);
    }
}
#endif

void leb_DecodeNodeAttributeArray(uint64_t heapID, inout float3 attributeArray[3])
{
    float3x3 m;
#if defined(LEB_MATRIX_CACHE_BINDING_SLOT)
    leb__DecodeTransformationMatrix_Tabulated(heapID, m);
#else
    leb__DecodeTransformationMatrix(heapID, m);
#endif
    for (int i = 0; i < 3; ++i)
    {
        float3 attributeVector = attributeArray[i];
        attributeArray[i][0] = dot(m[0], attributeVector);
        attributeArray[i][1] = dot(m[1], attributeVector);
        attributeArray[i][2] = dot(m[2], attributeVector);
    }
}

#if !defined(FP64_UNSUPPORTED)
void leb_DecodeNodeAttributeArray(uint64_t heapID, inout double3 attributeArray[3])
{
    double3x3 m;
#if defined(LEB_MATRIX_CACHE_BINDING_SLOT)
    leb__DecodeTransformationMatrix_Tabulated(heapID, m);
#else
    leb__DecodeTransformationMatrix(heapID, m);
#endif
    for (int i = 0; i < 3; ++i)
    {
        double3 attributeVector = attributeArray[i];
        attributeArray[i][0] = dot_double(m[0], attributeVector);
        attributeArray[i][1] = dot_double(m[1], attributeVector);
        attributeArray[i][2] = dot_double(m[2], attributeVector);
    }
}
#endif

/*******************************************************************************
 * DecodeNodeAttributeArray -- Compute the triangle attributes at the input node
 *
 */

void leb_DecodeNodeAttributeArray_parent_child(uint64_t heapID, inout float3 childAttribute[3], out float3 parentAttribute[3])
{
    float3x3 child, parent;
#if defined(LEB_MATRIX_CACHE_BINDING_SLOT)
    leb__DecodeTransformationMatrix_parent_child_Tabulated(heapID, parent, child);
#else
    leb__DecodeTransformationMatrix_parent_child(heapID, parent, child);
#endif
    int i;
    for (i = 0; i < 3; ++i)
    {
        float3 attributeVector = childAttribute[i];
        parentAttribute[i][0] = dot(parent[0], attributeVector);
        parentAttribute[i][1] = dot(parent[1], attributeVector);
        parentAttribute[i][2] = dot(parent[2], attributeVector);
    }

    for (i = 0; i < 3; ++i)
    {
        float3 attributeVector = childAttribute[i];
        childAttribute[i][0] = dot(child[0], attributeVector);
        childAttribute[i][1] = dot(child[1], attributeVector);
        childAttribute[i][2] = dot(child[2], attributeVector);
    }
}

#if !defined(FP64_UNSUPPORTED)
void leb_DecodeNodeAttributeArray_parent_child(uint64_t heapID, inout double3 childAttribute[3], out double3 parentAttribute[3])
{
    double3x3 child, parent;
#if defined(LEB_MATRIX_CACHE_BINDING_SLOT)
    leb__DecodeTransformationMatrix_parent_child_Tabulated(heapID, parent, child);
#else
    leb__DecodeTransformationMatrix_parent_child_optimized(heapID, parent, child);
#endif
    int i;
    for (i = 0; i < 3; ++i)
    {
        double3 attributeVector = childAttribute[i];
        parentAttribute[i][0] = dot_double(parent[0], attributeVector);
        parentAttribute[i][1] = dot_double(parent[1], attributeVector);
        parentAttribute[i][2] = dot_double(parent[2], attributeVector);
    }

    for (i = 0; i < 3; ++i)
    {
        double3 attributeVector = childAttribute[i];
        childAttribute[i][0] = dot_double(child[0], attributeVector);
        childAttribute[i][1] = dot_double(child[1], attributeVector);
        childAttribute[i][2] = dot_double(child[2], attributeVector);
    }
}
#endif

#endif // LEB_D_H