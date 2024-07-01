#ifndef BISECTOR_H
#define BISECTOR_H

// Pointer to an invalid neighbor or index
#define INVALID_POINTER 4294967295

// Possible culling state
#define BACK_FACE_CULLED -3
#define FRUSTUM_CULLED -2
#define TOO_SMALL -1
#define UNCHANGED_ELEMENT 0
#define BISECT_ELEMENT 1
#define SIMPLIFY_ELEMENT 2
#define MERGED_ELEMENT 3

// Bisector flags
#define VISIBLE_BISECTOR 0x1
#define MODIFIED_BISECTOR 0x2

struct BisectorData
{
    // Subvision that should be applied to this bisector
    uint32_t subdivisionPattern;

    // Allocated indices for this bisector
    uint32_t indices[3];

    // Neighbor that should be processed
    uint32_t problematicNeighbor;

    // State of this bisector (split, merge, etc)
    uint32_t bisectorState;

    // Visibility and modification flags of a bisector
    uint32_t flags;

    // ID used for the propagation
    uint32_t propagationID;
};

#if defined(UNSUPPORTED_FIRST_BIT_HIGH)
uint HeapIDDepth(uint64_t x)
{
    uint depth = 0;
    while (x > 0u) {
        ++depth;
        x >>= 1u;
    }
    return depth;
}
#else
uint HeapIDDepth(uint64_t heapID)
{
    return firstbithigh(int64_t(heapID)) + 1;
}
#endif

#endif // BISECTOR_H