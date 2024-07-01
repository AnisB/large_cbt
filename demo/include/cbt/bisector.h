#pragma once

// Project includes
#include <math/types.h>

// Pointer to an invalid neighbor or index
#define INVALID_POINTER UINT32_MAX

// Possible culling state
#define BACK_FACE_CULLED -3
#define FRUSTUM_CULLED -2
#define TOO_SMALL -1
#define UNCHANGED_ELEMENT 0
#define BISECT_ELEMENT 1
#define SIMPLIFY_ELEMENT 2
#define MERGED_ELEMENT 3

struct BisectorData
{
    // Subvision that should be applied to this bisector
    uint32_t subdivisionPattern;

    // Allocated indices for this bisector
    uint3 indices;

    // Neighbor that should be processed
    uint32_t problematicNeighbor;

    // State of this bisector (split, merge, etc)
    uint32_t bisectorState;

    // Visibility and modification flags of a bisector
    uint32_t flags;

    // ID used for the propagation
    uint32_t propagationID;
};
