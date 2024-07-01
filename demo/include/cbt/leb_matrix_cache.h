#pragma once

// Project includes
#include "graphics/types.h"

class LebMatrixCache
{
public:
    // Cst & Dst
    LebMatrixCache();
    ~LebMatrixCache();

    // Init & Release
    void intialize(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, uint32_t cacheDepth);
    void release();

    // Access the buffer
    GraphicsBuffer get_leb_matrix_buffer() const {return m_LebMatrixBuffer;}

private:
    GraphicsBuffer m_LebMatrixBuffer = 0;
    uint32_t m_CacheDepth = 0;
};