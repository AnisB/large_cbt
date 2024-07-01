
uint32_t OCBT_CLASS::num_elements() const
{
    return OCBT_NUM_ELEMENTS;
}

uint32_t OCBT_CLASS::last_level_size() const
{
    return OCBT_LAST_LEVEL_SIZE;
}

uint32_t OCBT_CLASS::max_depth() const
{
    return uint32_t(log2(OCBT_NUM_ELEMENTS));
}

uint32_t OCBT_CLASS::num_internal_buffers() const
{
    return 2;
}

char* OCBT_CLASS::raw_buffer(uint32_t bufferIdx)
{
    assert(bufferIdx < 2);
    return bufferIdx == 0 ? (char*)packed_heap : (char*)bitfield;
}

const char* OCBT_CLASS::raw_buffer(uint32_t bufferIdx) const
{
    assert(bufferIdx < 2);
    return bufferIdx == 0 ? (const char*)packed_heap : (const char*)bitfield;
}

// Buffer sizes
uint32_t OCBT_CLASS::buffer_size(uint32_t bufferIdx) const
{
    assert(bufferIdx < 2);
    return bufferIdx == 0 ? tree_memory_footprint() : bitfield_memory_footprint();
}

uint32_t OCBT_CLASS::element_size(uint32_t bufferIdx) const
{
    assert(bufferIdx < 2);
    return bufferIdx == 0 ? sizeof(uint32_t) : sizeof(uint64_t);
}

uint32_t OCBT_CLASS::memory_footprint() const
{
    return tree_memory_footprint() + bitfield_memory_footprint();
}

uint32_t OCBT_CLASS::tree_memory_footprint() const
{
    return OCBT_TREE_NUM_SLOTS * sizeof(uint32_t);
}

uint32_t OCBT_CLASS::bitfield_memory_footprint() const
{
    return (OCBT_NUM_ELEMENTS / 64) * sizeof(uint64_t);
}

void OCBT_CLASS::set_bit(uint32_t bitID, bool state)
{
    // Coordinates of the bit
    uint32_t slot = bitID / 64;
    uint32_t local_id = bitID % 64;

    if (state)
        bitfield[slot] |= 1ull << local_id;
    else
        bitfield[slot] &= ~(1ull << local_id);
}

uint32_t OCBT_CLASS::get_bit(uint32_t bitID) const
{
    uint32_t slot = bitID / 64;
    uint32_t local_id = bitID % 64;
    return (bitfield[slot] & (1ull << local_id)) >> local_id;
}

uint32_t OCBT_CLASS::bit_count() const
{
    return packed_heap[0];
}

uint32_t OCBT_CLASS::bit_count(uint32_t depth, uint32_t element) const
{
    return get_heap_element((1 << depth) + element);
}

uint32_t OCBT_CLASS::get_heap_element(uint32_t id) const
{
    // Figure out the location of the first bit of this element
    uint32_t real_heap_id = id - 1;
    uint32_t depth = uint32_t(log2(real_heap_id + 1));
    uint32_t level_first_element = (1 << depth) - 1;
    uint32_t id_in_level = real_heap_id - level_first_element;
    uint32_t first_bit = OCBT_depth_offset[depth] + OCBT_bit_count[depth] * id_in_level;
    if (depth < FIRST_VIRTUAL_LEVEL)
    {
        uint32_t slot = first_bit / 32;
        uint32_t local_id = first_bit % 32;
        uint32_t target_bits = (packed_heap[slot] >> local_id) & OCBT_bit_mask[depth];
        return (packed_heap[slot] >> local_id) & OCBT_bit_mask[depth];
    }
    else
    {
        uint32_t slot = first_bit / 64;
        uint32_t local_id = first_bit % 64;
        uint64_t target_bits = (bitfield[slot] >> local_id) & OCBT_bit_mask[depth];
        return countbits(target_bits);
    }
}

void OCBT_CLASS::set_heap_element(uint32_t id, uint32_t value)
{
    // Figure out the location of the first bit of this element
    uint32_t real_heap_id = id - 1;
    uint32_t depth = uint32_t(log2(real_heap_id + 1));
    uint32_t level_first_element = (1 << depth) - 1;
    uint32_t id_in_level = real_heap_id - level_first_element;

    // If this is the tree representation
    if (depth < FIRST_VIRTUAL_LEVEL)
    {
        // Find the slot and the local first bit
        uint32_t first_bit = OCBT_depth_offset[depth] + OCBT_bit_count[depth] * id_in_level;
        uint32_t slot = first_bit / 32;
        uint32_t local_id = first_bit % 32;

        // Extract the relevant bits
        uint32_t& target = packed_heap[slot];
        target &= ~(OCBT_bit_mask[depth] << (local_id));
        target |= (OCBT_bit_mask[depth] & value) << (local_id);
    }
    // Should be avoided, but is supported
    else if (depth == LEAF_LEVEL)
    {
        set_bit(id_in_level, value);
    }
    // Doesn't make sense
    else
    {
        assert(false);
    }
}

uint32_t OCBT_CLASS::decode_bit(uint32_t handle) const
{
#if NAIVE_DECODE
    uint32_t heapElementID = 1u;
    for (uint32_t currentDepth = 0; currentDepth < 17; ++currentDepth)
    {
        // Read the left element
        uint32_t heapValue = get_heap_element(2u * heapElementID);

        // Does it fall in the right or left subtree?
        uint32_t b = handle < heapValue ? 0u : 1u;

        // Pick a subtree
        heapElementID = 2u * heapElementID + b;

        // Move the iterator to exclude the right subtree if required
        handle -= heapValue * b;

    }
    return (heapElementID ^ OCBT_NUM_ELEMENTS);
#else
    uint32_t currentDepth = 0;
    uint32_t heapElementID = 1u;
    for (currentDepth = 0; currentDepth < FIRST_VIRTUAL_LEVEL; ++currentDepth)
    {
        // Read the left element
        uint32_t heapValue = get_heap_element(2u * heapElementID);

        // Does it fall in the right or left subtree?
        uint32_t b = handle < heapValue ? 0u : 1u;

        // Pick a subtree
        heapElementID = 2u * heapElementID + b;

        // Move the iterator to exclude the right subtree if required
        handle -= heapValue * b;
    }

    // Align with the internal depth
    currentDepth++;

    // Ok we have our subtree, now we need to pick the right bit
    uint64_t heapValue = bitfield[heapElementID - OCBT_LAST_LEVEL_SIZE * 2];

    for (; currentDepth < (LEAF_LEVEL + 1); ++currentDepth)
    {
        // Figure out the location of the first bit of this element
        uint32_t real_heap_id = 2 * heapElementID - 1;
        uint32_t level_first_element = (1 << currentDepth) - 1;
        uint32_t id_in_level = real_heap_id - level_first_element;
        uint32_t first_bit = OCBT_depth_offset[currentDepth] + OCBT_bit_count[currentDepth] * id_in_level;
        uint32_t local_id = first_bit % 64;
        uint64_t target_bits = (heapValue >> local_id) & OCBT_bit_mask[currentDepth];
        uint32_t heapValue = countbits(target_bits);

        // Does it fall in the right or left subtree?
        uint32_t b = handle < heapValue ? 0u : 1u;

        // Pick a subtree
        heapElementID = 2u * heapElementID + b;

        // Move the iterator to exclude the right subtree if required
        handle -= heapValue * b;
    }
    return (heapElementID ^ OCBT_NUM_ELEMENTS);
#endif
}

// decodes the position of the i-th zero in the bitfield
uint32_t OCBT_CLASS::decode_bit_complement(uint32_t handle) const
{
#if NAIVE_DECODE
    uint32_t bitID = 1u;
    uint32_t c = OCBT_NUM_ELEMENTS / 2u;

    while (bitID < OCBT_NUM_ELEMENTS) {
        uint32_t heapValue = c - get_heap_element(2u * bitID);
        uint32_t b = handle < heapValue ? 0u : 1u;

        bitID = 2u * bitID + b;
        handle -= heapValue * b;
        c /= 2u;
    }

    return (bitID ^ OCBT_NUM_ELEMENTS);
#else
    uint32_t heapElementID = 1u;
    uint32_t c = OCBT_NUM_ELEMENTS / 2u;
    uint32_t currentDepth = 0;

    for (currentDepth = 0; currentDepth < FIRST_VIRTUAL_LEVEL; ++currentDepth)
    {
        uint32_t heapValue = c - get_heap_element(2u * heapElementID);
        uint32_t b = handle < heapValue ? 0u : 1u;

        heapElementID = 2u * heapElementID + b;
        handle -= heapValue * b;
        c /= 2u;
    }

    // Align with the internal depth
    currentDepth++;

    // Ok we have our subtree, now we need to pick the right bit
    uint64_t heapValue = bitfield[heapElementID - OCBT_LAST_LEVEL_SIZE * 2];

    for (; currentDepth < (LEAF_LEVEL + 1); ++currentDepth)
    {
        // Figure out the location of the first bit of this element
        uint32_t real_heap_id = 2 * heapElementID - 1;
        uint32_t level_first_element = (1 << currentDepth) - 1;
        uint32_t id_in_level = real_heap_id - level_first_element;
        uint32_t first_bit = OCBT_depth_offset[currentDepth] + OCBT_bit_count[currentDepth] * id_in_level;
        uint32_t local_id = first_bit % 64;
        uint64_t target_bits = (heapValue >> local_id) & OCBT_bit_mask[currentDepth];
        uint32_t heapValue = c - countbits(target_bits);

        uint32_t b = handle < heapValue ? 0u : 1u;

        heapElementID = 2u * heapElementID + b;
        handle -= heapValue * b;
        c /= 2u;
    }

    return (heapElementID ^ OCBT_NUM_ELEMENTS);
#endif
}

void OCBT_CLASS::reduce()
{
    // First reduce the last level using countbits
    for (uint32_t threadID = 0; threadID < (OCBT_LAST_LEVEL_SIZE / 4); ++threadID)
    {
        // Initialize the packed sum
        uint32_t packedSum = 0;

        // Loop through the 2 pairs to process
        for (uint32_t pairIdx = 0; pairIdx < 4; ++pairIdx)
        {
            // First element of the pair
            uint32_t elementC = countbits(bitfield[threadID * 8 + 2 * pairIdx]);

            // Second element of the pair
            elementC += countbits(bitfield[threadID * 8 + 2 * pairIdx + 1]);

            // Store in the right bits
            packedSum |= (elementC << pairIdx * 8);
        }

        // Offset of the last level of the tree
        const uint32_t bufferOffset = OCBT_depth_offset[11] / 32;

        // Store the result into the bitfield
        packed_heap[bufferOffset + threadID] = packedSum;
    }

    // Then operate the reduction on the tree only (not the bitfield)
    for (uint32_t size = OCBT_NUM_ELEMENTS / 128u; size > 0u; size /= 2u) {
        uint32_t minHeapID = size;
        uint32_t maxHeapID = size * 2u;

        for (uint32_t heapID = minHeapID; heapID < maxHeapID; ++heapID)
        {
            uint32_t value = get_heap_element(2u * heapID) + get_heap_element(2u * heapID + 1u);
            set_heap_element(heapID, value);
        }
    }
}

void OCBT_CLASS::clear()
{
    memset(packed_heap, 0, OCBT_TREE_NUM_SLOTS * sizeof(uint32_t));
    memset(bitfield, 0, OCBT_BITFIELD_NUM_SLOTS * sizeof(uint64_t));
}

OCBT_CLASS::OCBT_CLASS()
{
    raw_memory = new uint32_t[OCBT_TREE_NUM_SLOTS + OCBT_NUM_ELEMENTS / 32];
    packed_heap = raw_memory;
    bitfield = (uint64_t*)(raw_memory + OCBT_TREE_NUM_SLOTS);
    clear();
}

OCBT_CLASS::~OCBT_CLASS()
{
    delete[] raw_memory;
}

void OCBT_CLASS::print()
{
    // Element Count
    std::cout << "Element Count " << OCBT_NUM_ELEMENTS << std::endl;

    // Binary tree
    uint32_t split_target = 1;
    uint32_t split_index = 0;
    std::cout << "Binary Tree " << std::endl;
    for (uint32_t idx = 0; idx < OCBT_NUM_ELEMENTS - 1; ++idx)
    {
        std::cout << get_heap_element(1 + idx) << " ";
        split_index++;
        if (split_index == split_target)
        {
            std::cout << std::endl;
            split_target *= 2;
            split_index = 0;
        }
    }
    std::cout << std::endl;

    // Bitfield
    std::cout << "Bitfield" << std::endl;
    for (uint32_t idx = 0; idx < OCBT_NUM_ELEMENTS; ++idx)
    {
        std::cout << get_bit(idx) << " ";
    }
    std::cout << std::endl;
}
