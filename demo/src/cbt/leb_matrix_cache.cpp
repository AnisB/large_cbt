// Project includes
#include "cbt/leb_matrix_cache.h"
#include "cbt/cbt_utility.h"
#include "graphics/dx12_backend.h"
#include "math/operators.h"

float3x3 SplittingMatrix(uint32_t bitValue)
{
    float b = float(bitValue);
    float c = 1.0f - b;

    return transpose(float3x3({
         0.0f,    b,    c,
         0.5f, 0.0f, 0.5f,
            b,    c, 0.0f }));
}

float3x3 DecodeSubdivisionMatrix(uint64_t heapID)
{
    float3x3 m = float3x3({ 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f });
    int32_t depth = find_msb_64(heapID) - 1;
    for (int32_t bitID = depth - 1; bitID >= 0; --bitID)
        m = mul(SplittingMatrix((heapID >> bitID) & 1u), m);
    return m;
}

LebMatrixCache::LebMatrixCache()
{
}

LebMatrixCache::~LebMatrixCache()
{
}

void LebMatrixCache::intialize(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, uint32_t cacheDepth)
{
    // Keep the cache depth
    m_CacheDepth = cacheDepth;
    uint32_t matrixCount = 2ULL << m_CacheDepth;

    // Create the runtime buffer
    m_LebMatrixBuffer = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(float3x3) * matrixCount, sizeof(float3x3), GraphicsBufferType::Default);

    // Build the CPU table
    std::vector<float3x3> table(matrixCount);
    table[0] = float3x3({ 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f });
    for (uint64_t heapID = 1ULL; heapID < (2ULL << m_CacheDepth); ++heapID)
        table[heapID] = DecodeSubdivisionMatrix(heapID);

    // Create the upload buffer
    GraphicsBuffer lebMatricesBufferUp = d3d12::graphics_resources::create_graphics_buffer(device, sizeof(float3x3) * matrixCount, sizeof(float3x3), GraphicsBufferType::Upload);
    d3d12::graphics_resources::set_buffer_data(lebMatricesBufferUp, (const char*)table.data(), sizeof(float3x3) * matrixCount);

    // Reset the command buffer
    d3d12::command_buffer::reset(cmdB);

    // Copy the input buffer to the processing buffers
    d3d12::command_buffer::copy_graphics_buffer(cmdB, lebMatricesBufferUp, m_LebMatrixBuffer);

    // Close the command buffer
    d3d12::command_buffer::close(cmdB);

    // Execute the command buffer in the command queue
    d3d12::command_queue::execute_command_buffer(cmdQ, cmdB);

    // Flush the queue
    d3d12::command_queue::flush(cmdQ);
}

void LebMatrixCache::release()
{
    d3d12::graphics_resources::destroy_graphics_buffer(m_LebMatrixBuffer);
    m_LebMatrixBuffer = 0;
}
