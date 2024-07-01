#ifndef OCBT_GENERIC_H
#define OCBT_GENERIC_H

// Declare the buffers we'll be using
RWStructuredBuffer<uint> _CBTBufferRW: register(CBT_BUFFER0_BINDING_SLOT);
RWStructuredBuffer<uint64_t> _BitfieldBufferRW: register(CBT_BUFFER1_BINDING_SLOT);

// Define the remaining values
#define BUFFER_ELEMENT_PER_LANE ((OCBT_TREE_NUM_SLOTS + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE)
#define BUFFER_ELEMENT_PER_LANE_NO_BITFIELD ((OCBT_TREE_NUM_SLOTS + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE)
#define BITFIELD_ELEMENT_PER_LANE ((OCBT_BITFIELD_NUM_SLOTS + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE)
#define WAVE_TREE_DEPTH uint(log2(OCBT_NUM_ELEMENTS))

uint32_t cbt_size()
{
    return OCBT_NUM_ELEMENTS;
}

uint32_t last_level_offset()
{
    return OCBT_depth_offset[TREE_LAST_LEVEL] / 32;
}

#if defined(CAN_USE_SHARED_MEMORY)
groupshared uint gs_cbtTree[OCBT_TREE_NUM_SLOTS];

// Function that sets a given bit
void set_bit(uint bitID, bool state)
{
    // Coordinates of the bit
    uint32_t slot = bitID / 64;
    uint32_t local_id = bitID % 64;

    if (state)
        _BitfieldBufferRW[slot] |= 1uLL << local_id;
    else
        _BitfieldBufferRW[slot] &= ~(1uLL << local_id);
}

void set_bit_atomic(uint bitID, bool state)
{
    // Coordinates of the bit
    uint32_t slot = bitID / 64;
    uint32_t local_id = bitID % 64;

    if (state)
        InterlockedOr(_BitfieldBufferRW[slot], 1uLL << local_id);
    else
        InterlockedAnd(_BitfieldBufferRW[slot], ~(1uLL << local_id));
}

uint get_bit(uint bitID)
{
    uint32_t slot = bitID / 64;
    uint32_t local_id = bitID % 64;
    return uint((_BitfieldBufferRW[slot] & (1uLL << local_id)) >> local_id);
}

uint get_heap_element(uint id)
{
    // Figure out the location of the first bit of this element
    uint32_t real_heap_id = id - 1;
    uint32_t depth = uint32_t(log2(real_heap_id + 1));
    uint32_t level_first_element = (1u << depth) - 1;
    uint32_t id_in_level = real_heap_id - level_first_element;
    uint32_t first_bit = OCBT_depth_offset[depth] + OCBT_bit_count[depth] * id_in_level;
    if (depth < FIRST_VIRTUAL_LEVEL)
    {
        uint32_t slot = first_bit / 32;
        uint32_t local_id = first_bit % 32;
        uint32_t target_bits = (gs_cbtTree[slot] >> local_id) & uint32_t(OCBT_bit_mask[depth]);
        return (gs_cbtTree[slot] >> local_id) & uint32_t(OCBT_bit_mask[depth]);
    }
    else
    {
        uint32_t slot = first_bit / 64;
        uint32_t local_id = first_bit % 64;
        uint64_t target_bits = (_BitfieldBufferRW[slot] >> local_id) & OCBT_bit_mask[depth];
        return countbits(target_bits);
    }
}

// Should not be called if depth > TREE_LAST_LEVEL
void set_heap_element(uint id, uint value)
{
    // Figure out the location of the first bit of this element
    uint real_heap_id = id - 1;
    uint depth = uint(log2(real_heap_id + 1));
    uint level_first_element = (1u << depth) - 1;
    uint first_bit = OCBT_depth_offset[depth] + OCBT_bit_count[depth] * (real_heap_id - level_first_element);

    // Find the slot and the local first bit
    uint slot = first_bit / 32;
    uint local_id = first_bit % 32;

    // Extract the relevant bits
    gs_cbtTree[slot] &= ~(uint32_t(OCBT_bit_mask[depth]) << local_id);
    gs_cbtTree[slot] |= ((uint32_t(OCBT_bit_mask[depth]) & value) << local_id);
}

// Should not be called if depth > TREE_LAST_LEVEL
void set_heap_element_atomic(uint id, uint value)
{
    // Figure out the location of the first bit of this element
    uint real_heap_id = id - 1;
    uint depth = uint(log2(real_heap_id + 1));
    uint level_first_element = (1u << depth) - 1;
    uint first_bit = OCBT_depth_offset[depth] + OCBT_bit_count[depth] * (real_heap_id - level_first_element);

    // Find the slot and the local first bit
    uint slot = first_bit / 32;
    uint local_id = first_bit % 32;

    // Extract the relevant bits
    InterlockedAnd(gs_cbtTree[slot], ~(uint32_t(OCBT_bit_mask[depth]) << local_id));
    InterlockedOr(gs_cbtTree[slot], ((uint32_t(OCBT_bit_mask[depth]) & value) << local_id));
}

// Function that returns the number of active bits
uint bit_count()
{
    return gs_cbtTree[0];
}

uint bit_count(uint depth, uint element)
{
    return get_heap_element((1u << depth) + element);
}

// decodes the position of the i-th one in the bitfield
uint decode_bit(uint handle)
{
#if defined(NAIVE_DECODE)
    uint bitID = 1;
    for (uint currentDepth = 0; currentDepth < WAVE_TREE_DEPTH; ++currentDepth)
    {
        uint heapValue = get_heap_element(2 * bitID);
        uint b = handle < heapValue ? 0 : 1;

        bitID = 2 * bitID + b;
        handle -= heapValue * b;
    }

    return (bitID ^ OCBT_NUM_ELEMENTS);
#else
    uint currentDepth = 0;
    uint heapElementID = 1u;
    for (currentDepth = 0; currentDepth < FIRST_VIRTUAL_LEVEL; ++currentDepth)
    {
        // Read the left element
        uint heapValue = get_heap_element(2u * heapElementID);

        // Does it fall in the right or left subtree?
        uint b = handle < heapValue ? 0u : 1u;

        // Pick a subtree
        heapElementID = 2u * heapElementID + b;

        // Move the iterator to exclude the right subtree if required
        handle -= heapValue * b;
    }

    // Align with the internal depth
    currentDepth++;

    // Ok we have our subtree, now we need to pick the right bit
    uint64_t heapValue = _BitfieldBufferRW[heapElementID - OCBT_LAST_LEVEL_SIZE * 2];
    uint64_t mask = 0xffffffff;
    uint32_t bitCount = 32;
    for (; currentDepth < (WAVE_TREE_DEPTH + 1); ++currentDepth)
    {
        // Figure out the location of the first bit of this element
        uint real_heap_id =  2 * heapElementID - 1;
        uint level_first_element = (1u << currentDepth) - 1;
        uint id_in_level = real_heap_id - level_first_element;
        uint first_bit = bitCount * id_in_level;
        uint local_id = first_bit % 64;
        uint64_t target_bits = (heapValue >> local_id) & mask;
        uint heapValue = countbits(target_bits);

        // Does it fall in the right or left subtree?
        uint b = handle < heapValue ? 0u : 1u;

        // Pick a subtree
        heapElementID = 2u * heapElementID + b;

        // Move the iterator to exclude the right subtree if required
        handle -= heapValue * b;

        // Adjust the mask and bitcount
        bitCount /= 2;
        mask = mask >> bitCount;
    }
    return (heapElementID ^ OCBT_NUM_ELEMENTS);
#endif
}

// decodes the position of the i-th zero in the bitfield
uint decode_bit_complement(uint handle)
{
#if defined(NAIVE_DECODE)
    uint bitID = 1u;
    uint c = OCBT_NUM_ELEMENTS / 2u;

    while (bitID < OCBT_NUM_ELEMENTS) {
        uint heapValue = c - get_heap_element(2u * bitID);
        uint b = handle < heapValue ? 0u : 1u;

        bitID = 2u * bitID + b;
        handle -= heapValue * b;
        c /= 2u;
    }

    return (bitID ^ OCBT_NUM_ELEMENTS);
#else
    uint heapElementID = 1u;
    uint c = OCBT_NUM_ELEMENTS / 2u;
    uint currentDepth = 0;

    for (currentDepth = 0; currentDepth < FIRST_VIRTUAL_LEVEL; ++currentDepth)
    {
        uint heapValue = c - get_heap_element(2u * heapElementID);
        uint b = handle < heapValue ? 0u : 1u;

        heapElementID = 2u * heapElementID + b;
        handle -= heapValue * b;
        c /= 2u;
    }

    // Align with the internal depth
    currentDepth++;

    // Ok we have our subtree, now we need to pick the right bit
    uint64_t heapValue = _BitfieldBufferRW[heapElementID - OCBT_LAST_LEVEL_SIZE * 2];
    uint64_t mask = 0xffffffff;
    uint32_t bitCount = 32;
    for (; currentDepth < (WAVE_TREE_DEPTH + 1); ++currentDepth)
    {
        // Figure out the location of the first bit of this element
        uint real_heap_id = 2 * heapElementID - 1;
        uint level_first_element = (1u << currentDepth) - 1;
        uint id_in_level = real_heap_id - level_first_element;
        uint first_bit = bitCount * id_in_level;
        uint local_id = first_bit % 64;
        uint64_t target_bits = (heapValue >> local_id) & mask;
        uint heapValue = c - countbits(target_bits);

        uint b = handle < heapValue ? 0u : 1u;

        heapElementID = 2u * heapElementID + b;
        handle -= heapValue * b;
        c /= 2u;

        // Adjust the mask and bitcount
        bitCount /= 2;
        mask = mask >> bitCount;
    }

    return (heapElementID ^ OCBT_NUM_ELEMENTS);
#endif
}

void reduce(uint groupIndex)
{
	// First we do a reduction until each lane has exactly one element to process
	uint initial_pass_size = OCBT_NUM_ELEMENTS / WORKGROUP_SIZE;
    for (uint it = initial_pass_size / 64, offset = OCBT_NUM_ELEMENTS / 64; it > 0 ; it >>=1, offset >>=1)
    {
        uint minHeapID = offset + (groupIndex * it);
        uint maxHeapID = offset + ((groupIndex + 1) * it);

        for (uint heapID = minHeapID; heapID < maxHeapID; ++heapID)
        {
            set_heap_element(heapID, get_heap_element(heapID * 2) + get_heap_element(heapID * 2 + 1));
        }
    }
	GroupMemoryBarrierWithGroupSync();

	for(uint s = WORKGROUP_SIZE / 2; s > 0u; s >>= 1)
    {
        if (groupIndex < s)
        {
            uint v = s + groupIndex;
            set_heap_element(v, get_heap_element(v * 2) + get_heap_element(v * 2 + 1));
        }
        GroupMemoryBarrierWithGroupSync();
    }
}

void reduce_prepass(uint dispatchThreadID)
{
    // Initialize the packed sum
    uint packedSum = 0;

    // Loop through the 4 pairs to process
    for (uint pairIdx = 0; pairIdx < 4; ++pairIdx)
    {
        // First element of the pair
        uint elementC = countbits(_BitfieldBufferRW[dispatchThreadID * 8 + 2 * pairIdx]);

        // Second element of the pair
        elementC += countbits(_BitfieldBufferRW[dispatchThreadID * 8 + 2 * pairIdx + 1]);

        // Store in the right bits
        packedSum |= (elementC << pairIdx * 8);
    }

    // Offset of the last level of the tree
    const uint bufferOffset = last_level_offset();

    // Store the result into the bitfield
    _CBTBufferRW[bufferOffset + dispatchThreadID] = packedSum;
}

void reduce_second_pass(uint groupIndex)
{
    // Load the lowest level (and only the last level)
    const uint level0Offset = OCBT_depth_offset[9] / 32;
    for (uint e = 0; e < 4; ++e)
    {
        uint target_element = 4 * groupIndex + e;
        gs_cbtTree[level0Offset + target_element] = _CBTBufferRW[level0Offset + target_element];
    }
    GroupMemoryBarrierWithGroupSync();

    // First we do a reduction until each lane has exactly one element to process
    uint initial_pass_size = 256;
    for (uint it = initial_pass_size / 64, offset = initial_pass_size; it > 0 ; it >>=1, offset >>=1)
    {
        uint minHeapID = offset + (groupIndex * it);
        uint maxHeapID = offset + ((groupIndex + 1) * it);

        for (uint heapID = minHeapID; heapID < maxHeapID; ++heapID)
        {
            set_heap_element(heapID, get_heap_element(heapID * 2) + get_heap_element(heapID * 2 + 1));
        }
    }
    GroupMemoryBarrierWithGroupSync();
    
    for(uint s = WORKGROUP_SIZE / 2; s > 0u; s >>= 1)
    {
        if (groupIndex < s)
        {
            uint v = s + groupIndex;
            set_heap_element(v, get_heap_element(v * 2) + get_heap_element(v * 2 + 1));
        }
        GroupMemoryBarrierWithGroupSync();
    }

    // Make sure all the previous operations are done
    GroupMemoryBarrierWithGroupSync();

    // Load the bitfield to the LDS
    for (uint e = 0; e < 5; ++e)
    {
        uint target_element = 5 * groupIndex + e;
        if (target_element < 319)
            _CBTBufferRW[target_element] = gs_cbtTree[target_element];
    }
}

void reduce_no_bitfield(uint groupIndex)
{
    // First we do a reduction until each lane has exactly one element to process
    uint initial_pass_size = OCBT_NUM_ELEMENTS / WORKGROUP_SIZE;
    for (uint it = initial_pass_size / 128, offset = OCBT_NUM_ELEMENTS / 128; it > 0 ; it >>=1, offset >>=1)
    {
        uint minHeapID = offset + (groupIndex * it);
        uint maxHeapID = offset + ((groupIndex + 1) * it);

        for (uint heapID = minHeapID; heapID < maxHeapID; ++heapID)
        {
            set_heap_element(heapID, get_heap_element(heapID * 2) + get_heap_element(heapID * 2 + 1));
        }
    }
    GroupMemoryBarrierWithGroupSync();

    for(uint s = WORKGROUP_SIZE / 2; s > 0u; s >>= 1)
    {
        if (groupIndex < s)
        {
            uint v = s + groupIndex;
            set_heap_element(v, get_heap_element(v * 2) + get_heap_element(v * 2 + 1));
        }
        GroupMemoryBarrierWithGroupSync();
    }
}

void clear_cbt(uint groupIndex)
{
    for (uint v = 0; v < BUFFER_ELEMENT_PER_LANE; ++v)
    {
        uint target_element = BUFFER_ELEMENT_PER_LANE * groupIndex + v;
        if (target_element < OCBT_TREE_NUM_SLOTS)
            gs_cbtTree[target_element] = 0;
    }

    for (uint b = 0; b < BITFIELD_ELEMENT_PER_LANE; ++b)
    {
        uint target_element = BITFIELD_ELEMENT_PER_LANE * groupIndex + b;
        if (target_element < OCBT_BITFIELD_NUM_SLOTS)
            _BitfieldBufferRW[target_element] = 0;
    }
    GroupMemoryBarrierWithGroupSync();
}

// Importante note
// Depending on your target GPU architecture, the pattern used to load has a different performance behavior
// here is the best performant based on our tests:
// NVIDIA uint target_element = groupIndex + WORKGROUP_SIZE * e;
// AMD uint target_element = BUFFER_ELEMENT_PER_LANE * groupIndex + e;

void load_buffer_to_shared_memory(uint groupIndex)
{
    // Load the bitfield to the LDS
    for (uint e = 0; e < BUFFER_ELEMENT_PER_LANE; ++e)
    {
        uint target_element = BUFFER_ELEMENT_PER_LANE * groupIndex + e;
        if (target_element < OCBT_TREE_NUM_SLOTS)
            gs_cbtTree[target_element] = _CBTBufferRW[target_element];
    }
    GroupMemoryBarrierWithGroupSync();
}

void load_shared_memory_to_buffer(uint groupIndex)
{
    // Make sure all the previous operations are done
    GroupMemoryBarrierWithGroupSync();

    // Load the bitfield to the LDS
    for (uint e = 0; e < BUFFER_ELEMENT_PER_LANE; ++e)
    {
        uint target_element = BUFFER_ELEMENT_PER_LANE * groupIndex + e;
        if (target_element < OCBT_TREE_NUM_SLOTS)
            _CBTBufferRW[target_element] = gs_cbtTree[target_element];
    }
}
#endif // CAN_USE_SHARED_MEMORY

void set_bit_atomic_buffer(uint bitID, bool state)
{
    // Coordinates of the bit
    uint32_t slot = bitID / 64;
    uint32_t local_id = bitID % 64;

    if (state)
        InterlockedOr(_BitfieldBufferRW[slot], 1uLL << local_id);
    else
        InterlockedAnd(_BitfieldBufferRW[slot], ~(1uLL << local_id));
}

uint32_t bit_count_buffer()
{
    return _CBTBufferRW[0];
}

#endif // OCBT_GENERIC_H
