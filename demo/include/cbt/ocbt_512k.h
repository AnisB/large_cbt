#pragma once

// Project includes
#include "cbt/cbt.h"

// Number of elements
#define OCBT_512K_NUM_ELEMENTS 524288

class UPCBT_512k : public CBT
{
public:
    // Cst & Dst
    UPCBT_512k();
    ~UPCBT_512k();

    // Num elements & Size
    uint32_t num_elements() const;
    uint32_t last_level_size() const;
    uint32_t max_depth() const;

    // The number of internal buffers used to store the cbt
    uint32_t num_internal_buffers() const;

    // Raw view of the internal buffers
    char* raw_buffer(uint32_t bufferIdx = 0);
    const char* raw_buffer(uint32_t bufferIdx = 0) const;

    // Buffer sizes
    uint32_t buffer_size(uint32_t bufferIdx = 0) const;
    uint32_t element_size(uint32_t bufferIdx = 0) const;

    // Memory footprint of the CBT
    uint32_t memory_footprint() const;
    uint32_t tree_memory_footprint() const;
    uint32_t bitfield_memory_footprint() const;

    // Bit manipulation
    void set_bit(uint32_t bitID, bool state);
    uint32_t get_bit(uint32_t bitID) const;

    // Bit counting
    uint32_t bit_count() const;
    uint32_t bit_count(uint32_t depth, uint32_t element) const;

    // Tree traversal
    uint32_t decode_bit(uint32_t handle) const;
    uint32_t decode_bit_complement(uint32_t handle) const;

    // Heap Manipulation
    uint32_t get_heap_element(uint32_t id) const;
    void set_heap_element(uint32_t id, uint32_t value);

    // Other operation
    void reduce();
    void clear();

    // Debug
    void print();
protected:
    uint32_t* raw_memory;
    uint32_t* packed_heap;
    uint64_t* bitfield;
};
