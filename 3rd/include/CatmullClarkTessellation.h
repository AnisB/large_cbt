#ifndef CCT_INCLUDE_CCT_H
#define CCT_INCLUDE_CCT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CCT_STATIC
#define CCTDEF static
#else
#define CCTDEF extern
#endif

typedef struct {
    int32_t id;
    int32_t depth;
} cct_Bisector;

typedef union {
    struct {uint32_t h0, h1, h2;};
    uint32_t array[3];
} cct_BisectorHalfedgeIDs;

typedef union {
    struct {int32_t n0, n1, n2;};
    int32_t array[3];
} cct_BisectorNeighborIDs;

// cbt_Tree factory
CCTDEF cbt_Tree *cct_Create(const cc_Subd *subd);

// accessors
CCTDEF int32_t cct_RootBisectorCount(const cc_Subd *subd);
CCTDEF int32_t cct_BisectorCount(const cbt_Tree *cbt, const cc_Subd *subd);
CCTDEF int32_t cct_BisectorDepth(const cbt_Node node, const cc_Subd *subd);

// conversion routines
CCTDEF cct_Bisector cct_NodeToBisector(const cbt_Node node, const cc_Subd *subd);
CCTDEF cbt_Node cct_BisectorToNode(const cct_Bisector bisector, const cc_Subd *subd);

// decoding routines
CCTDEF cct_BisectorHalfedgeIDs cct_DecodeHalfedgeIDs(cct_Bisector b, const cc_Subd *subd);
CCTDEF cct_BisectorNeighborIDs cct_DecodeNeighborIDs(cct_Bisector b, const cc_Subd *subd);

// split / merge
CCTDEF void cct_Split(cbt_Tree *cbt,
                      const cct_Bisector bisector,
                      const cc_Subd *subd);

#ifdef __cplusplus
} // extern "C"
#endif

//
//
//// end header file ///////////////////////////////////////////////////////////
#endif // CCT_INCLUDE_CCT_H


/*******************************************************************************
 * LgPowerOfTwo -- Find the log base 2 of an integer that is a power of two
 *
 * See:
 * https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
 *
 */
static inline uint32_t cct__LgPowerOfTwo(uint32_t x)
{
    uint32_t r = (x & 0xAAAAAAAAu) != 0u;

    r |= ((x & 0xFFFF0000u) != 0) << 4;
    r |= ((x & 0xFF00FF00u) != 0) << 3;
    r |= ((x & 0xF0F0F0F0u) != 0) << 2;
    r |= ((x & 0xCCCCCCCCu) != 0) << 1;

    return r;
}


/*******************************************************************************
 * RoundUpToPowerOfTwo -- Computes the next highest power of 2 for a 32-bit integer
 *
 * See:
 * https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 *
 */
static uint32_t cct__RoundUpToPowerOfTwo(uint32_t x)
{
    x--;
    x|= x >>  1;
    x|= x >>  2;
    x|= x >>  4;
    x|= x >>  8;
    x|= x >> 16;
    x++;

    return x;
}


/*******************************************************************************
 * RootBisectorCount -- returns the number of faces at root
 *
 */
CCTDEF int32_t cct_RootBisectorCount(const cc_Subd *subd)
{
    return ccm_HalfedgeCount(subd->cage);
}


/*******************************************************************************
 * MinCbtDepth -- returns the minimum depth required for the CBT to work
 *
 */
static int32_t cct__MinCbtDepth(const cc_Subd *subd)
{
    const uint32_t rootBisectorCount = (uint32_t)cct_RootBisectorCount(subd);

    return (int32_t)cct__LgPowerOfTwo(cct__RoundUpToPowerOfTwo(rootBisectorCount));
}


/*******************************************************************************
 * NullBisectorCount -- returns the number of unused bisectors created in the CBT
 *
 */
static int32_t cct__NullBisectorCount(const cc_Subd *subd)
{
    return (1 << cct__MinCbtDepth(subd)) - cct_RootBisectorCount(subd);
}


/*******************************************************************************
 * MaxBisectorDepth -- returns the maximum bisection depth supported by the subd
 *
 */
static int32_t cct__MaxBisectorDepth(const cc_Subd *subd)
{
    return (ccs_MaxDepth(subd) * 2) - 1;
}


/*******************************************************************************
 * Create -- Creates a CBT suitable for computing the tessellation
 *
 */
CCTDEF cbt_Tree *cct_Create(const cc_Subd *subd)
{
    const int32_t minDepth = cct__MinCbtDepth(subd);
    const int32_t cbtDepth = minDepth + cct__MaxBisectorDepth(subd);

    return cbt_CreateAtDepth(cbtDepth, minDepth);
}


/*******************************************************************************
 * BisectorCount -- Returns the number of bisectors in the tessellation
 *
 */
CCTDEF int32_t cct_BisectorCount(const cbt_Tree *cbt, const cc_Subd *subd)
{
    return cbt_NodeCount(cbt) - cct__NullBisectorCount(subd);
}


/*******************************************************************************
 * NodeToBisector -- Converts a cbt_Node to a cct_Bisector
 *
 */
CCTDEF cct_Bisector cct_NodeToBisector(const cbt_Node node, const cc_Subd *subd)
{
    const int32_t depth = node.depth - cct__MinCbtDepth(subd);
    const int32_t bitMask = 1u << node.depth;

    return (cct_Bisector){node.id ^ bitMask, depth};
}


/*******************************************************************************
 * BisectorToNode -- Converts a cct_Bisector to a cbt_Node
 *
 */
static cbt_Node
cct_BisectorToNode_Fast(const cct_Bisector bisector, const int32_t minCbtDepth)
{
    const int32_t depth = bisector.depth + minCbtDepth;
    const int32_t bitMask = 1u << depth;

    return (cbt_Node){bisector.id | bitMask, depth};
}
CCTDEF cbt_Node cct_BisectorToNode(const cct_Bisector bisector, const cc_Subd *subd)
{
    return cct_BisectorToNode_Fast(bisector, cct__MinCbtDepth(subd));
}


/*******************************************************************************
 * GetBitValue -- Returns the value of a bit stored in a 32-bit word
 *
 */
static uint32_t cct__GetBitValue(const uint32_t bitField, int32_t bitID)
{
    return ((bitField >> bitID) & 1u);
}


/*******************************************************************************
 * EvenRule -- Even half-edge splitting rule
 *
 */
void cct__EvenRule(uint32_t *x, int32_t b)
{
    if (b == 0) {
        x[2] = (x[0] | 2);
        x[1] = (x[0] | 1);
        x[0] = (x[0] | 0);
    } else {
        x[0] = (x[2] | 2);
        x[1] = (x[2] | 3);
        x[2] = (x[2] | 0);
    }
}


/*******************************************************************************
 * EvenRule -- Odd half-edge splitting rule
 *
 */
void cct__OddRule(uint32_t *x, int32_t b)
{
    if (b == 0) {
        x[2] = (x[1] << 2) | 0;
        x[1] = (x[0] << 2) | 2;
        x[0] = (x[0] << 2) | 0;
    } else {
        x[0] = (x[1] << 2) | 0;
        x[1] = (x[1] << 2) | 2;
        x[2] = (x[2] << 2) | 0;
    }
}


/*******************************************************************************
 * DecodeHalfedgeIDs -- Retrieve the halfedges associated with the bisector
 *
 */
CCTDEF cct_BisectorHalfedgeIDs
cct_DecodeHalfedgeIDs(cct_Bisector bisector, const cc_Subd *subd)
{
    const cc_Mesh *cage = subd->cage;
    const int32_t halfedgeID = bisector.id >> bisector.depth;
    const int32_t nextID = ccm_HalfedgeNextID(cage, halfedgeID);
    const int32_t h0 = 4 * halfedgeID + 0;
    const int32_t h1 = 4 * halfedgeID + 2;
    const int32_t h2 = 4 * nextID;
    cct_BisectorHalfedgeIDs halfedgeIDs = (cct_BisectorHalfedgeIDs){h0, h1, h2};
    int32_t isEven = true;

    for (int32_t bitID = bisector.depth - 1; bitID >= 0; --bitID) {
        const uint32_t bitValue = cct__GetBitValue(bisector.id, bitID);

        if (isEven) {
            cct__EvenRule(halfedgeIDs.array, bitValue);
        } else {
            cct__OddRule(halfedgeIDs.array, bitValue);
        }

        isEven= !isEven;
    }

    // swap winding for odd levels
    if (!isEven) {
        int32_t tmp = halfedgeIDs.array[0];

        halfedgeIDs.array[0] = halfedgeIDs.array[2];
        halfedgeIDs.array[2] = tmp;
    }

    return halfedgeIDs;
}


/*******************************************************************************
 * BisectNeighborIDs -- Computes new neighborhood after one subdivision step
 *
 */
cct_BisectorNeighborIDs
cct__BisectNeighborIDs(
    const cct_BisectorNeighborIDs neighborIDs,
    const int32_t bisectorID,
    uint32_t bitValue
) {
    const int32_t b = bisectorID;
    const int32_t n0 = neighborIDs.n0,
                  n1 = neighborIDs.n1,
                  n2 = neighborIDs.n2;

    if (bitValue == 0) {
        return (cct_BisectorNeighborIDs){2 * n2 + 1, 2 * b + 1, 2 * n0 + 1};
    } else {
        return (cct_BisectorNeighborIDs){2 * n1 + 0, 2 * n0 + 0, 2 * b + 0};
    }
}


/*******************************************************************************
 * DecodeNeighborIDs -- Computes bisector neighborhood
 *
 */
cct_BisectorNeighborIDs
cct_DecodeNeighborIDs(cct_Bisector bisector, const cc_Subd *subd)
{
    const cc_Mesh *cage = subd->cage;
    const int32_t halfedgeID = bisector.id >> bisector.depth;
    const int32_t n0 = ccm_HalfedgeTwinID(cage, halfedgeID);
    const int32_t n1 = ccm_HalfedgeNextID(cage, halfedgeID);
    const int32_t n2 = ccm_HalfedgePrevID(cage, halfedgeID);
    cct_BisectorNeighborIDs neighborIDs = (cct_BisectorNeighborIDs){n0, n1, n2};

    for (int32_t bitID = bisector.depth - 1; bitID >= 0; --bitID) {
        const int32_t bisectorID = bisector.id >> bitID;

        neighborIDs = cct__BisectNeighborIDs(neighborIDs,
                                             bisectorID >> 1,
                                             bisectorID & 1);
    }

    return neighborIDs;
}

/*******************************************************************************
 * SplitNode -- Bisects a triangle in the current tessellation
 *
 */
CCTDEF void
cct_Split(cbt_Tree *cbt, const cct_Bisector bisector, const cc_Subd *subd)
{
    const int32_t minCbtDepth = cct__MinCbtDepth(subd);
    cct_Bisector iterator = bisector;

    cbt_SplitNode_Fast(cbt, cct_BisectorToNode(iterator, subd));
    iterator.id = cct_DecodeNeighborIDs(iterator, subd).n0;

    while (iterator.id >= 0 && iterator.depth >= 0) {
        cbt_SplitNode_Fast(cbt, cct_BisectorToNode_Fast(iterator, minCbtDepth));
        iterator.id>>= 1; iterator.depth-= 1; // parent bisector
        cbt_SplitNode_Fast(cbt, cct_BisectorToNode_Fast(iterator, minCbtDepth));
        iterator.id = cct_DecodeNeighborIDs(iterator, subd).n0;
    }
}