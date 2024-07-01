#ifndef UPDATE_UTILITIES_H
#define UPDATE_UTILITIES_H

// Includes
#include "shader_lib/bisector.hlsl"

// Bisector buffers
RWStructuredBuffer<uint64_t> _HeapIDBuffer: register(HEAP_ID_BUFFER_BINDING_SLOT);
RWStructuredBuffer<uint3> _NeighborsBuffer: register(NEIGHBORS_BUFFER_BINDING_SLOT);
RWStructuredBuffer<uint3> _NeighborsOutputBuffer: register(NEIGHBORS_OUTPUT_BUFFER_BINDING_SLOT);

// Intermediate buffers
RWStructuredBuffer<BisectorData> _BisectorDataBuffer: register(BISECTOR_DATA_BUFFER_BINDING_SLOT);
RWStructuredBuffer<uint> _ClassificationBuffer: register(CLASSIFICATION_BUFFER_BINDING_SLOT);
RWStructuredBuffer<uint> _SimplificationBuffer: register(SIMPLIFICATION_BUFFER_BINDING_SLOT);
RWStructuredBuffer<int> _AllocateBuffer: register(ALLOCATE_BUFFER_BINDING_SLOT);
RWStructuredBuffer<int> _PropagateBuffer: register(PROPAGATE_BUFFER_BINDING_SLOT);
RWStructuredBuffer<int> _MemoryBuffer: register(MEMORY_BUFFER_BINDING_SLOT);

// Indexation buffers
RWStructuredBuffer<uint> _IndirectDispatchBuffer: register(INDIRECT_DISPATCH_BUFFER_BINDING_SLOT);
RWStructuredBuffer<uint> _IndirectDrawBuffer: register(INDIRECT_DRAW_BUFFER_BINDING_SLOT);
RWStructuredBuffer<uint> _BisectorIndicesBuffer: register(BISECTOR_INDICES_BINDING_SLOT);
RWStructuredBuffer<uint> _VisibleBisectorIndicesBuffer: register(VISIBLE_BISECTOR_INDICES_BINDING_SLOT);
RWStructuredBuffer<uint> _ModifiedBisectorIndicesBuffer: register(MODIFIED_BISECTOR_INDICES_BINDING_SLOT);

// Debug buffers
RWStructuredBuffer<uint> _ValidationBuffer: register(VALIDATION_BUFFER_BINDING_SLOT);

// Possible splits
#define NO_SPLIT 0x00
#define CENTER_SPLIT 0x01
#define RIGHT_SPLIT 0x02
#define LEFT_SPLIT 0x04
#define RIGHT_DOUBLE_SPLIT (CENTER_SPLIT | RIGHT_SPLIT)
#define LEFT_DOUBLE_SPLIT (CENTER_SPLIT | LEFT_SPLIT)
#define TRIPLE_SPLIT (CENTER_SPLIT | RIGHT_SPLIT | LEFT_SPLIT)

// Split buffer slots
#define SPLIT_COUNTER 0
#define SIMPLIFY_COUNTER 1
#define CLASSIFY_COUNTER_OFFSET 2

void ResetBuffers()
{
    _MemoryBuffer[0] = 0;
    _MemoryBuffer[1] = cbt_size() - bit_count_buffer();

    _ClassificationBuffer[SPLIT_COUNTER] = 0;
    _ClassificationBuffer[SIMPLIFY_COUNTER] = 0;

    _AllocateBuffer[0] = 0;

    _PropagateBuffer[0] = 0;
    _PropagateBuffer[1] = 0;

    _SimplificationBuffer[0] = 0;

    _IndirectDrawBuffer[0] = 0;
    _IndirectDrawBuffer[1] = 1;
    _IndirectDrawBuffer[2] = 0;
    _IndirectDrawBuffer[3] = 0;

    _IndirectDrawBuffer[4] = 0;
    _IndirectDrawBuffer[5] = 1;
    _IndirectDrawBuffer[6] = 0;
    _IndirectDrawBuffer[7] = 0;

    _IndirectDrawBuffer[8] = 0;
}

void ClassifyElement(uint currentID, BisectorGeometry bis, uint totalNumElements, uint baseDepth)
{
    // Evaluate the depth of the element
    uint64_t heapID = _HeapIDBuffer[currentID];
    uint depth = HeapIDDepth(heapID);
    BisectorData cbisectorData = _BisectorDataBuffer[currentID];

    // Reset some values
    cbisectorData.subdivisionPattern = 0;
    cbisectorData.bisectorState = UNCHANGED_ELEMENT;
    cbisectorData.problematicNeighbor = INVALID_POINTER;
    cbisectorData.flags = VISIBLE_BISECTOR;
    
    // Does this triangle intersect the circle?
    int currentValidity = ClassifyBisector(bis, depth);
    if (currentValidity > UNCHANGED_ELEMENT)
    {
        // This element should be bisected
        int targetSlot;
        cbisectorData.bisectorState = BISECT_ELEMENT;
        InterlockedAdd(_ClassificationBuffer[SPLIT_COUNTER], 1, targetSlot);
        _ClassificationBuffer[CLASSIFY_COUNTER_OFFSET + targetSlot] = currentID;

    }
    else
        cbisectorData.flags = currentValidity >= TOO_SMALL ? VISIBLE_BISECTOR : 0;

    // What's the validity of the father?
    if (baseDepth != depth && currentValidity < UNCHANGED_ELEMENT)
    {   
        // Mark that it requires simplification
        cbisectorData.bisectorState = SIMPLIFY_ELEMENT;

        // Only register it if it has an even heapID, the odd ones will be processed by the even ones
        if (heapID % 2 == 0)
        {
            int targetSlot;
            InterlockedAdd(_ClassificationBuffer[SIMPLIFY_COUNTER], 1, targetSlot);
            _ClassificationBuffer[CLASSIFY_COUNTER_OFFSET + totalNumElements + targetSlot] = currentID;
        }
    }

    // Update the bisector data
    _BisectorDataBuffer[currentID] = cbisectorData;
}

void SplitElement(uint currentID, uint baseDepth)
{
    // Get the neighbors information
    uint3 cNeighbors = _NeighborsBuffer[currentID];

    // If there is a neighbor X
    if (cNeighbors.x != INVALID_POINTER)
    {
        // This is on the path of it's neighbor X
        uint3 xNeighbors = _NeighborsBuffer[cNeighbors.x];
        if (xNeighbors.z == currentID && _BisectorDataBuffer[cNeighbors.x].bisectorState != UNCHANGED_ELEMENT)
            return;
    }

    // If there is a neighbor Y
    if (cNeighbors.y != INVALID_POINTER)
    {
        // This is on the path of it's neighbor Y
        uint3 yNeighbors = _NeighborsBuffer[cNeighbors.y];
        if (yNeighbors.z == currentID && _BisectorDataBuffer[cNeighbors.y].bisectorState != UNCHANGED_ELEMENT)
            return;
    }

    // Depth of the current triangle
    uint64_t heapID = _HeapIDBuffer[currentID];
    uint currentDepth = HeapIDDepth(heapID);

    // Compute the maximal required memory for this subdivision
    int maxRequiredMemory = 2 * (currentDepth - baseDepth) - 1;

    // Get the twin information
    uint twinID = cNeighbors.z;

    // This avoid the massive over-reservation and saves a bunch of artifacts
    if (twinID == INVALID_POINTER)
        maxRequiredMemory = 1;
    else if (_NeighborsBuffer[twinID].z == currentID)
        maxRequiredMemory = 2;

    // Try to reserve
    int remainingMemory;
    InterlockedAdd(_MemoryBuffer[1], -maxRequiredMemory, remainingMemory);

    // Did someone manage to sneak-in while we were trying to pick the memory, add it back and try again
    if (remainingMemory < maxRequiredMemory)
    {
        // Then add back the required memory and stop
        InterlockedAdd(_MemoryBuffer[1], maxRequiredMemory, remainingMemory);
        return;
    }

    // Let's actually count the memory that we will be using
    uint usedMemory = 1;
    uint prevPattern;
    InterlockedOr(_BisectorDataBuffer[currentID].subdivisionPattern, CENTER_SPLIT, prevPattern);

    // If this is not zero, it means an other neighbor went faster than us, we restore the memory and leave.
    if (prevPattern != 0)
    {
        InterlockedAdd(_MemoryBuffer[1], maxRequiredMemory, remainingMemory);
        return;
    }

    // Mark this for allocation
    uint targetLocation;
    InterlockedAdd(_AllocateBuffer[0], 1, targetLocation);
    _AllocateBuffer[1 + targetLocation] = currentID;

    // While we're not done (up the tree or everything is subdivided properly)
    bool done = false;
    while (!done)
    {
        // If this neighbor is not allocated, we're done.
        if (twinID == INVALID_POINTER)
            break;

        // Grab the bisector of the neighbor
        uint64_t nHeapID = _HeapIDBuffer[twinID];
        BisectorData nBisectorData = _BisectorDataBuffer[twinID];
        uint nDepth = HeapIDDepth(nHeapID);
        uint3 nNeighbors = _NeighborsBuffer[twinID];

        // If both triangles have the same depth
        if (nDepth == currentDepth)
        {
            // Raised the center split
            InterlockedOr(_BisectorDataBuffer[twinID].subdivisionPattern, CENTER_SPLIT, prevPattern);

            // Only account for it if it was not raised before.
            if (prevPattern == 0)
            {
                // Mark this for allocation
                uint targetLocation;
                InterlockedAdd(_AllocateBuffer[0], 1, targetLocation);
                _AllocateBuffer[1 + targetLocation] = twinID;
                usedMemory++;
            }

            // And we're done
            done = true;
        }
        // If this node has already been subdivided, it means that we need to add the third subdivision and we're done
        else
        {
            if (nNeighbors[0] == currentID)
                InterlockedOr(_BisectorDataBuffer[twinID].subdivisionPattern, RIGHT_DOUBLE_SPLIT, prevPattern);
            else // if (nNeighbors[1] == currentID)
                InterlockedOr(_BisectorDataBuffer[twinID].subdivisionPattern, LEFT_DOUBLE_SPLIT, prevPattern);

            if (prevPattern != 0)
            {
                usedMemory++;
                done = true;
            }
            else
            {
                // Mark this for allocation
                uint targetLocation;
                InterlockedAdd(_AllocateBuffer[0], 1, targetLocation);
                _AllocateBuffer[1 + targetLocation] = twinID;
                
                // Account for two splits
                usedMemory += 2;

                // the new bisector that needs to be propagated
                currentID = twinID;
                currentDepth = nDepth;
                twinID = _NeighborsBuffer[currentID].z;
            }
        }
    }

    // Add back the unused memory (in case)
    InterlockedAdd(_MemoryBuffer[1], max(maxRequiredMemory - usedMemory, 0), remainingMemory);
}

void AllocateElement(uint currentID)
{
    // Load the bisector for this element
    BisectorData bisectorData = _BisectorDataBuffer[currentID];

    // Does this guy need to be subdivided
    if (bisectorData.subdivisionPattern != 0)
    {
        // How many bits do we need?
        int numSlots = countbits(bisectorData.subdivisionPattern);

        // Request the number of bits we need using an interlock add
        uint firstBitIndex = 0;
        InterlockedAdd(_MemoryBuffer[0], numSlots, firstBitIndex);

        // llocate the bits we need
        for (uint bitId = 0; bitId < numSlots; ++bitId)
        {
            uint index = decode_bit_complement(firstBitIndex + bitId);
            bisectorData.indices[bitId] = index;
        }

        // Output 
        _BisectorDataBuffer[currentID] = bisectorData;
    }
}

#define SUBLING0_ID 0
#define SUBLING1_ID 1
#define SUBLING2_ID 2
void evaluate_neighbors(uint currentID, uint bisectorID, out uint resX, out uint resY)
{
    BisectorData nBisectorData = _BisectorDataBuffer[bisectorID];
    uint3 nNeighbors = _NeighborsBuffer[bisectorID];
    if (nBisectorData.subdivisionPattern == 0x01)
    {
        resX = nBisectorData.indices[SUBLING0_ID];
        resY = bisectorID;
    }
    else if (nBisectorData.subdivisionPattern == 0x03)
    {
        if (nNeighbors[0] == currentID)
        {
            resX = nBisectorData.indices[SUBLING1_ID];
            resY = bisectorID;
        }
        else
        {
            resX = nBisectorData.indices[SUBLING0_ID];
            resY = nBisectorData.indices[SUBLING1_ID];
        }
    }
    else if (nBisectorData.subdivisionPattern == 0x05)
    {
        if (nNeighbors[1] == currentID)
        {
            resX = nBisectorData.indices[SUBLING1_ID];
            resY = nBisectorData.indices[SUBLING0_ID];
        }
        else
        {
            resX = nBisectorData.indices[SUBLING0_ID];
            resY = bisectorID;
        }
    }
    else
    {
        if (nNeighbors[0] == currentID)
        {
            resX = nBisectorData.indices[SUBLING1_ID];
            resY = bisectorID;
        }
        else if (nNeighbors[1] == currentID)
        {
            resX = nBisectorData.indices[SUBLING2_ID];
            resY = nBisectorData.indices[SUBLING0_ID];
        }
        else
        {
            resX = nBisectorData.indices[SUBLING0_ID];
            resY = nBisectorData.indices[SUBLING1_ID];
        }
    }
}

void BisectElement(uint currentID)
{
    // If this bisector is not allocated or not subdivided, stop right away
    uint64_t baseHeapID = _HeapIDBuffer[currentID];
    BisectorData cBisectorData = _BisectorDataBuffer[currentID];
    if (baseHeapID == 0 || cBisectorData.subdivisionPattern == NO_SPLIT)
        return;

    // Load the bisector data of the target triangle
    uint currentSubdiv = cBisectorData.subdivisionPattern;

    // neighbors of the parent
    uint3 cNeighbors = _NeighborsBuffer[currentID];
    uint p_n0 = cNeighbors[0];
    uint p_n1 = cNeighbors[1];
    uint p_n2 = cNeighbors[2];

    // Get the main axis subdiv
    uint siblingID0 = cBisectorData.indices[0];
    uint siblingID1 = cBisectorData.indices[1];
    uint siblingID2 = cBisectorData.indices[2];

    // Simple subdivision (along the main axis)
    if (currentSubdiv == CENTER_SPLIT)
    {   
        uint resX = INVALID_POINTER, resY = INVALID_POINTER;
        if (p_n2 != INVALID_POINTER)
            evaluate_neighbors(currentID, p_n2, resX, resY);

        // Set the heap IDs
        _HeapIDBuffer[currentID] = 2 * baseHeapID;
        _HeapIDBuffer[siblingID0] = 2 * baseHeapID + 1;

        // Update the neighbors
        uint3 modifiedNeighbors;
        modifiedNeighbors[0] = siblingID0;
        modifiedNeighbors[1] = resX;
        modifiedNeighbors[2] = p_n0;
        _NeighborsOutputBuffer[currentID] = modifiedNeighbors;
        modifiedNeighbors[0] = resY;
        modifiedNeighbors[1] = currentID;
        modifiedNeighbors[2] = p_n1;
        _NeighborsOutputBuffer[siblingID0] = modifiedNeighbors;

        // Keep track of the parent
        BisectorData modifiedBisector = cBisectorData;
        modifiedBisector.propagationID = currentID;

        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        _BisectorDataBuffer[currentID] = modifiedBisector;

        modifiedBisector.problematicNeighbor = p_n1;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        _BisectorDataBuffer[siblingID0] = modifiedBisector;

        // Mark this for propagation
        uint targetLocation;
        InterlockedAdd(_PropagateBuffer[0], 1, targetLocation);
        _PropagateBuffer[2 + targetLocation] = siblingID0;
    }
    else if (currentSubdiv == RIGHT_DOUBLE_SPLIT)
    {
        // Grab the bisector of the twin
        uint res0X = INVALID_POINTER, res0Y = INVALID_POINTER;
        evaluate_neighbors(currentID, p_n0, res0X, res0Y);

        uint res1X = INVALID_POINTER, res1Y = INVALID_POINTER;
        if (p_n2 != INVALID_POINTER)
            evaluate_neighbors(currentID, p_n2, res1X, res1Y);

        // Set the heap IDs
        _HeapIDBuffer[currentID] = 4 * baseHeapID;
        _HeapIDBuffer[siblingID0] = 2 * baseHeapID + 1;
        _HeapIDBuffer[siblingID1] = 4 * baseHeapID + 1;

        uint3 modifiedNeighbors;
        modifiedNeighbors[0] = siblingID1;
        modifiedNeighbors[1] = res0X;
        modifiedNeighbors[2] = siblingID0;
        _NeighborsOutputBuffer[currentID] = modifiedNeighbors;
        modifiedNeighbors[0] = res1Y;
        modifiedNeighbors[1] = currentID;
        modifiedNeighbors[2] = p_n1;
        _NeighborsOutputBuffer[siblingID0] = modifiedNeighbors;
        modifiedNeighbors[0] = res0Y;
        modifiedNeighbors[1] = currentID;
        modifiedNeighbors[2] = res1X;
        _NeighborsOutputBuffer[siblingID1] = modifiedNeighbors;

        // Keep track of the parent
        BisectorData modifiedBisector = cBisectorData;
        modifiedBisector.propagationID = currentID;

        // Lower the element down the tree and update it's sibling
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        _BisectorDataBuffer[currentID] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = p_n1;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        _BisectorDataBuffer[siblingID0] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        _BisectorDataBuffer[siblingID1] = modifiedBisector;

        // Mark this for propagation
        uint targetLocation;
        InterlockedAdd(_PropagateBuffer[0], 1, targetLocation);
        _PropagateBuffer[2 + targetLocation] = siblingID0;
    }
    else if (currentSubdiv == LEFT_DOUBLE_SPLIT)
    {
        // Grab the bisector of the twin
        uint res0X = INVALID_POINTER, res0Y = INVALID_POINTER;
        evaluate_neighbors(currentID, p_n1, res0X, res0Y);

        uint res1X = INVALID_POINTER, res1Y = INVALID_POINTER;
        if (p_n2 != INVALID_POINTER)
            evaluate_neighbors(currentID, p_n2, res1X, res1Y);

        // Set the heap IDs
        _HeapIDBuffer[currentID] = 2 * baseHeapID;
        _HeapIDBuffer[siblingID0] = 4 * baseHeapID + 2;
        _HeapIDBuffer[siblingID1] = 4 * baseHeapID + 3;

        uint3 modifiedNeighbors;
        modifiedNeighbors[0] = siblingID1;
        modifiedNeighbors[1] = res1X;
        modifiedNeighbors[2] = p_n0;
        _NeighborsOutputBuffer[currentID] = modifiedNeighbors;
        modifiedNeighbors[0] = siblingID1;
        modifiedNeighbors[1] = res0X;
        modifiedNeighbors[2] = res1Y;
        _NeighborsOutputBuffer[siblingID0] = modifiedNeighbors;
        modifiedNeighbors[0] = res0Y;
        modifiedNeighbors[1] = siblingID0;
        modifiedNeighbors[2] = currentID;
        _NeighborsOutputBuffer[siblingID1] = modifiedNeighbors;

        // Keep track of the parent
        BisectorData modifiedBisector = cBisectorData;
        modifiedBisector.propagationID = currentID;

        // Lower the element down the tree and update it's sibling
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        _BisectorDataBuffer[currentID] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        _BisectorDataBuffer[siblingID0] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        _BisectorDataBuffer[siblingID1] = modifiedBisector;
    }
    else if (currentSubdiv == TRIPLE_SPLIT)
    {
        // Grab the bisector of the twin
        uint res0X = INVALID_POINTER, res0Y = INVALID_POINTER;
        evaluate_neighbors(currentID, p_n0, res0X, res0Y);

        uint res1X = INVALID_POINTER, res1Y = INVALID_POINTER;
        evaluate_neighbors(currentID, p_n1, res1X, res1Y);

        uint res2X = INVALID_POINTER, res2Y = INVALID_POINTER;
        if (p_n2 != INVALID_POINTER)
            evaluate_neighbors(currentID, p_n2, res2X, res2Y);

        // Set the heap IDs
        _HeapIDBuffer[currentID] = 4 * baseHeapID;
        _HeapIDBuffer[siblingID0] = 4 * baseHeapID + 2;
        _HeapIDBuffer[siblingID1] = 4 * baseHeapID + 1;
        _HeapIDBuffer[siblingID2] = 4 * baseHeapID + 3;

        uint3 modifiedNeighbors;
        modifiedNeighbors[0] = siblingID1;
        modifiedNeighbors[1] = res0X;
        modifiedNeighbors[2] = siblingID2;
        _NeighborsOutputBuffer[currentID] = modifiedNeighbors;
        modifiedNeighbors[0] = siblingID2;
        modifiedNeighbors[1] = res1X;
        modifiedNeighbors[2] = res2Y;
        _NeighborsOutputBuffer[siblingID0] = modifiedNeighbors;
        modifiedNeighbors[0] = res0Y;
        modifiedNeighbors[1] = currentID;
        modifiedNeighbors[2] = res2X;
        _NeighborsOutputBuffer[siblingID1] = modifiedNeighbors;
        modifiedNeighbors[0] = res1Y;
        modifiedNeighbors[1] = siblingID0;
        modifiedNeighbors[2] = currentID;
        _NeighborsOutputBuffer[siblingID2] = modifiedNeighbors;

        // Keep track of the parent
        BisectorData modifiedBisector = cBisectorData;
        modifiedBisector.propagationID = currentID;

        // Lower the element down the tree and update it's sibling
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        _BisectorDataBuffer[currentID] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        _BisectorDataBuffer[siblingID0] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        _BisectorDataBuffer[siblingID1] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        _BisectorDataBuffer[siblingID2] = modifiedBisector;
    }

    // How many bits do we need to raise
    uint numSiblings = countbits(currentSubdiv);
    for (uint siblingIdx = 0; siblingIdx < numSiblings; ++siblingIdx)
    {
        set_bit_atomic_buffer(cBisectorData.indices[siblingIdx], true);
    }
}

void PropagateBisectElement(uint currentID)
{
    // Load the bisector data of the target triangle
    BisectorData cBisectorData = _BisectorDataBuffer[currentID];

    // neighbors of the parent
    uint parentID = cBisectorData.propagationID;
    uint problematicNeighbor = cBisectorData.problematicNeighbor;

    // Read the neighbor that may have changed
    BisectorData tBisectorData = _BisectorDataBuffer[problematicNeighbor];
    uint3 tNeighbors = _NeighborsBuffer[problematicNeighbor];
    uint targetID = problematicNeighbor;
    uint sibling1 = tBisectorData.indices[1];

    if (tBisectorData.subdivisionPattern == NO_SPLIT)
    {
        if (tNeighbors[0] == parentID)
            _NeighborsBuffer[targetID][0] = currentID;
        if (tNeighbors[1] == parentID)
            _NeighborsBuffer[targetID][1] = currentID;
        if (tNeighbors[2] == parentID)
            _NeighborsBuffer[targetID][2] = currentID;
    }
    else if (tBisectorData.subdivisionPattern == CENTER_SPLIT)
    {
        if (_NeighborsBuffer[targetID][2] == parentID)
            _NeighborsBuffer[targetID][2] = currentID;
        if (_NeighborsBuffer[tBisectorData.propagationID][2] == parentID)
            _NeighborsBuffer[tBisectorData.propagationID][2] = currentID;
    }
    else if (tBisectorData.subdivisionPattern == RIGHT_DOUBLE_SPLIT)
    {
        _NeighborsBuffer[sibling1][2] = currentID;
    } 
    else if (tBisectorData.subdivisionPattern == LEFT_DOUBLE_SPLIT)
    {
        _NeighborsBuffer[targetID][2] = currentID;
    }

    // Reset the problematic neighbor and the bisection state
    _BisectorDataBuffer[currentID].problematicNeighbor = INVALID_POINTER;
    _BisectorDataBuffer[currentID].bisectorState = UNCHANGED_ELEMENT;
}

void PrepareSimplifyElement(uint currentID)
{
    // Get the bisector
    BisectorData cBisectorData = _BisectorDataBuffer[currentID];

    // Grab the current bisector
    uint64_t cHeapID = _HeapIDBuffer[currentID];

    // If this is not an even heap number it will be handeled by it's pair, the twin or the twin's pair

    // Neighbors of this element
    uint3 cNeighbors = _NeighborsBuffer[currentID];

    // Evaluate the depth of this bisector
    uint currentDepth = HeapIDDepth(cHeapID);

    // Grab the pair neighbor (it has to exist)
    uint pairID = cNeighbors[0];
    uint64_t pHeapID = _HeapIDBuffer[pairID];
    BisectorData pBisectorData = _BisectorDataBuffer[pairID];
    uint3 pNeighbors = _NeighborsBuffer[pairID];

    // Evaluate the depth of the pair
    uint pairDepth = HeapIDDepth(pHeapID);

    // If they are not at the same depth or the pair is not to be simplified, we're done
    if (pairDepth != currentDepth || pBisectorData.bisectorState != SIMPLIFY_ELEMENT)
        return;

    // We need to identify our twin pair
    uint twinLowID = pNeighbors[0];
    uint twinHighID = cNeighbors[1];
    if (twinLowID != INVALID_POINTER)
    {
        // Grab the two bisectors
        uint64_t twinLowHeapID = _HeapIDBuffer[twinLowID];
        uint64_t twinHighHeapID = _HeapIDBuffer[twinHighID];
        
        // The current bisector is not the smallest element of the neighborhood, he will be handeled by twinLowBisect if needed
        if (cHeapID > twinLowHeapID)
            return;

        // Compute the depth of both neighbors
        uint lowFacingDepth = HeapIDDepth(twinLowHeapID);
        uint highFacingDepth = HeapIDDepth(twinHighHeapID);

        // If all four elements are not on the same
        if (lowFacingDepth != currentDepth || highFacingDepth != currentDepth)
            return;

        // Grab the two bisectors
        BisectorData twinLowBisectData = _BisectorDataBuffer[twinLowID];
        BisectorData twinHighBisectData = _BisectorDataBuffer[twinHighID];

        // This element should not be doing the simplifications if:
        // - One of the four elements doesn't have the same depth
        // - One of the four elements isn't flagged for simplification
        if (twinLowBisectData.bisectorState != SIMPLIFY_ELEMENT
            || twinHighBisectData.bisectorState != SIMPLIFY_ELEMENT)
            return;
    }

    // This element will simplify itself, it's pair and possibilty it's twin and twin-pair.
    uint bisectorSlot;
    InterlockedAdd(_SimplificationBuffer[0], 1, bisectorSlot);

    // Log the bisector ID
    _SimplificationBuffer[1 + bisectorSlot] = currentID;
}

void SimplifyElement(uint currentID)
{
    // Grab the current bisector
    BisectorData cBisectorData = _BisectorDataBuffer[currentID];
    uint3 cNeighbors = _NeighborsBuffer[currentID];

    // Grab the pair neighbor (it has to exist)
    uint pairID = cNeighbors[0];
    BisectorData pBisectorData = _BisectorDataBuffer[pairID];
    uint3 pNeighbors = _NeighborsBuffer[pairID];

    // We need to indentify our twin pair
    uint twinLowID = pNeighbors[0];
    uint twinHighID = cNeighbors[1];

    // Set the heap IDs
    _HeapIDBuffer[currentID] = _HeapIDBuffer[currentID] / 2;
    _HeapIDBuffer[pairID] = 0;

    // All conditions are met for us to simplify these triangles
    uint3 newNeighbors;
    newNeighbors[0] = cNeighbors[2];
    newNeighbors[1] = pNeighbors[2];
    newNeighbors[2] = twinLowID;
    _NeighborsBuffer[currentID] = newNeighbors;

    // Update the bisector data
    cBisectorData.propagationID = pairID;
    cBisectorData.problematicNeighbor = pNeighbors[2];
    cBisectorData.bisectorState = MERGED_ELEMENT;
    cBisectorData.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
    _BisectorDataBuffer[currentID] = cBisectorData;

    // Mark this for propagation
    if (cBisectorData.problematicNeighbor != INVALID_POINTER)
    {
        // Mark this for propagation
        uint targetLocation;
        InterlockedAdd(_PropagateBuffer[1], 1, targetLocation);
        _PropagateBuffer[2 + targetLocation] = currentID;
    }

    // Clear the pair's heap for identification
    pBisectorData.bisectorState = MERGED_ELEMENT;
    pBisectorData.flags = 0;
    _BisectorDataBuffer[pairID] = pBisectorData;

    // Don't forget to free the bit
    set_bit_atomic_buffer(pairID, false);

    // If there was a facing pair, simplify it aswell
    if (twinLowID != INVALID_POINTER)
    {
        // Set the heap IDs
        _HeapIDBuffer[twinLowID] = _HeapIDBuffer[twinLowID] / 2;
        _HeapIDBuffer[twinHighID] = 0;

        // Read both bisectors
        BisectorData lowFacingBst = _BisectorDataBuffer[twinLowID];
        uint3 lfNeighbors = _NeighborsBuffer[twinLowID];
        BisectorData highFacingBst = _BisectorDataBuffer[twinHighID];
        uint3 hfNeighbors = _NeighborsBuffer[twinHighID];

        // Update the lowest ID
        newNeighbors[0] = lfNeighbors[2];
        newNeighbors[1] = hfNeighbors[2];
        newNeighbors[2] = currentID;
        _NeighborsBuffer[twinLowID] = newNeighbors;

        // Update the twin bisector data
        lowFacingBst.propagationID = twinHighID;
        lowFacingBst.problematicNeighbor = hfNeighbors[2];
        lowFacingBst.bisectorState = MERGED_ELEMENT;
        lowFacingBst.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        _BisectorDataBuffer[twinLowID] = lowFacingBst;

        if (lowFacingBst.problematicNeighbor != INVALID_POINTER)
        {
            // Mark this for propagation
            uint targetLocation;
            InterlockedAdd(_PropagateBuffer[1], 1, targetLocation);
            _PropagateBuffer[2 + targetLocation] = twinLowID;
        }

        // Clear the pair's heap for identification 
        highFacingBst.bisectorState = MERGED_ELEMENT;
        highFacingBst.flags = 0;
        _BisectorDataBuffer[twinHighID] = highFacingBst;

        // Don't forget to free the bit
        set_bit_atomic_buffer(twinHighID, false);
    }
}

void PropagateElementSimplify(uint currentID)
{
    // Load the bisector data of the target element
    BisectorData cBisectorData = _BisectorDataBuffer[currentID];

    // Id of the element before the simplification
    uint deletedPair = cBisectorData.propagationID;

    // neighbors of the parent
    uint neighborID = cBisectorData.problematicNeighbor;

    // Read the neighbor that may have changed
    BisectorData nBisectorData = _BisectorDataBuffer[neighborID];
    uint3 nNeighbors = _NeighborsBuffer[neighborID];

    // The neighbor has not changed, so we just need to make it point on currentID instead of the pair that was deleted
    if (nBisectorData.bisectorState != MERGED_ELEMENT)
    {
        for (uint i = 0; i < 3; ++i)
        {
            if (nNeighbors[i] == deletedPair)
                _NeighborsBuffer[neighborID][i] = currentID;
        }
    }
    // The neighbor has had a simplification, so we need to update a different neighbor based on if it went up one depth in the tree or was deleted.
    else if (nBisectorData.bisectorState == MERGED_ELEMENT)
    {
        // He still exist, but was simplified
        if (_HeapIDBuffer[neighborID] != 0)
        {
            for (uint i = 0; i < 3; ++i)
            {
                if (nNeighbors[i] == deletedPair)
                    _NeighborsBuffer[neighborID][i] = currentID;
            }
        }
        // He is gone, we need to update his pair instead of him.
        else
        {
            uint neighborPair = nNeighbors[1];
            for (uint i = 0; i < 3; ++i)
            {
                if (_NeighborsBuffer[neighborPair][i] == deletedPair)
                    _NeighborsBuffer[neighborPair][i] = currentID;
            }
        }
    }

    // Reset the problematic neighbor
    _BisectorDataBuffer[currentID].problematicNeighbor = INVALID_POINTER;
}

void BisectorElementIndexation(uint currentID)
{
    // Grab the current heap ID
    uint64_t cHeapID = _HeapIDBuffer[currentID];

    // Deallocated element, we don't care
    if (cHeapID == 0)
        return;

    // Reserve a slot for this bisector
    uint bisectorSlot;
    InterlockedAdd(_IndirectDrawBuffer[0], 3, bisectorSlot);

    // Keep track of it's global ID
    _BisectorIndicesBuffer[bisectorSlot / 3] = currentID;

    // Load the bisector data of the target element
    BisectorData cBisectorData = _BisectorDataBuffer[currentID];
    
    // Is it visible?
    if ((cBisectorData.flags & VISIBLE_BISECTOR) == 0)
        return;

    // Reserve a slot for this visible bisector
    InterlockedAdd(_IndirectDrawBuffer[4], 3, bisectorSlot);

    // Keep track of it's global ID
    _VisibleBisectorIndicesBuffer[bisectorSlot / 3] = currentID;

    // Is it visible?
    if ((cBisectorData.flags & MODIFIED_BISECTOR) == 0)
        return;

    // Reserve a slot for this visible bisector
    InterlockedAdd(_IndirectDrawBuffer[8], 4, bisectorSlot);

    // Keep track of it's global ID
    _ModifiedBisectorIndicesBuffer[bisectorSlot / 4] = currentID;
}

void ValidateBisector(uint currentID)
{
    // Grab the current heap ID
    uint64_t cHeapID = _HeapIDBuffer[currentID];

    // Deallocated element, we don't care
    if (cHeapID == 0)
        return;

    // Load the bisector data of the target element
    uint3 cNeighbors = _NeighborsBuffer[currentID];

    bool failed = false;
    uint targetNeighbor = INVALID_POINTER;
    uint targetIdx = INVALID_POINTER;
    for (uint i = 0; i < 3; ++i)
    {
        uint neighborID = cNeighbors[i];
        if (neighborID != INVALID_POINTER)
        {
            bool found = false;
            uint3 nNeighbors = _NeighborsBuffer[neighborID];
            for (uint j = 0; j < 3; ++j)
            {
                if (nNeighbors[j] == currentID)
                    found = true;
            }
            if (!found)
            {
                failed = true;
                targetNeighbor = neighborID;
                targetIdx = i;
                break;
            }
        }
    }
    
    // Notify the failure
    if (failed)
    {
        uint prevValue;
        InterlockedAdd(_ValidationBuffer[0], 1, prevValue);
    }    
}

#endif // UPDATE_UTILITIES_H