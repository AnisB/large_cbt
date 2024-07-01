#pragma once

// Project includes
#include <graphics/types.h>

class CBT
{
public:
    // The number of elements in the bitfield
    virtual uint32_t num_elements() const = 0;

    // Size of the last level of the tree before the bitfield
    virtual uint32_t last_level_size() const = 0;

    // Maximal depth of the tree
    virtual uint32_t max_depth() const = 0;

    // The number of internal buffers used to store the cbt
    virtual uint32_t num_internal_buffers() const = 0;

    // Raw view of the internal buffers
    virtual char* raw_buffer(uint32_t bufferIdx = 0) = 0;
    virtual const char* raw_buffer(uint32_t bufferIdx = 0) const = 0;

    // Buffer sizes
    virtual uint32_t buffer_size(uint32_t bufferIdx = 0) const = 0;
    virtual uint32_t element_size(uint32_t bufferIdx) const = 0;

    // Memory footprint of the cbt
    virtual uint32_t memory_footprint() const = 0;
    virtual uint32_t tree_memory_footprint() const = 0;
    virtual uint32_t bitfield_memory_footprint() const = 0;

    // Bit manipulation
    virtual void set_bit(uint32_t bitID, bool state) = 0;
    virtual uint32_t get_bit(uint32_t bitID) const = 0;

    // Bit counting
    virtual uint32_t bit_count() const = 0;
    virtual uint32_t bit_count(uint32_t depth, uint32_t element) const = 0;

    // Tree traversal
    virtual uint32_t decode_bit(uint32_t handle) const = 0;
    virtual uint32_t decode_bit_complement(uint32_t handle) const = 0;

    // Heap Manipulation
    virtual uint32_t get_heap_element(uint32_t id) const = 0;
    virtual void set_heap_element(uint32_t id, uint32_t value) = 0;

    // Other operations
    virtual void reduce() = 0;
    virtual void clear() = 0;

    // Debug
    virtual void print() = 0;
};

// CBT representation on a graphics device
struct GPU_CBT
{
    // Num elements that the CBT can hold
    uint32_t numElements = 0;

    // Last level size 
    uint32_t lastLevelSize = 0;

    // Number of Graphics buffers used to represent the CBT
    uint32_t bufferCount = 0;

    // Graphics buffer used to store CBT
    GraphicsBuffer bufferArray[2];
};

// Create the device version of the cbt
void initialize_gpu_cbt(const CBT& cbt, GraphicsDevice device, CommandQueue queue, CommandBuffer buffer, GPU_CBT& gpuCBT);
void release_gpu_cbt(GPU_CBT& gpuCBT);
