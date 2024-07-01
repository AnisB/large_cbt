// Project includes
#include "cbt/cbt_utility.h"
#include "cbt/OCBT_1m.h"
#include "math/operators.h"
#include "tools/security.h"

// System includes
#include <stdlib.h>
#include <iostream>

/*
Level 0: 32 bit // [0, 1048576] x 1, needs a minimum of 21 bits (rounded up to 32 for alignment and required for atomic operations)
Level 1: 32 bit // [0, 524288] x 2, needs a minimum of 20 bits (rounded up to 32 for alignment and required for atomic operations)
Level 2: 32 bit // [0, 262144] x 4, needs a minimum of 19 bits (rounded up to 32 for alignment and required for atomic operations)
Level 3: 32 bit // [0, 131072] x 8, needs a minimum of 18 bits (rounded up to 32 for alignment and required for atomic operations)
Level 4: 32 bit // [0, 65536] x 16, needs a minimum of 17 bits (rounded up to 32 for alignment and required for atomic operations)
Level 5: 32 bit // [0, 32768] x 32, needs a minimum of 16 bits (bumped to 32 bits for atomic operations)
Level 6: 32 bit // [0, 16384] x 64, needs a minimum of 15 bits (rounded up to 16 for alignment and bumped to 32 bits for atomic operations)


Level 7: 32 bit // [0, 8192] x 128, needs a minimum of 14 bits (rounded up to 16 for alignment and bumped to 32 bits for atomic operations)
Level 8: 16 bit // [0, 4096] x 256, needs a minimum of 13 bits (rounded up to 16 for alignment and bumped to 32 bits for atomic operations)
Level 9: 16 bit // [0, 2048] x 512, needs a minimum of 12 bits (rounded up to 16 for alignment)
Level 10: 16 bit // [0, 1024] x 1024, needs a minimum of 11 bits (rounded up to 16 for alignment)
Level 11: 16 bit // [0, 512] x 2048, needs a minimum of 10 bits (rounded up to 16 for alignment)
Level 12: 16 bit // [0, 256] x 4096, needs a minimum of 9 bits (rounded up to 16 for alignment)
Level 13: 8 bit // [0, 128] x 8192, needs a minimum of 8 bits

Level 14: Raw 64 bits representation
*/

// Tree sizes
#define OCBT_TREE_SIZE_BITS (32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32 + 32 * 64 + 16 * 128 + 16 * 256 + 16 * 512 + 16 * 1024 + 16* 2048 + 16 * 4096 + 8 * 8192)
#define OCBT_TREE_NUM_SLOTS (OCBT_TREE_SIZE_BITS / 32)
#define OCBT_BITFIELD_NUM_SLOTS (OCBT_NUM_ELEMENTS / 64)
#define OCBT_LAST_LEVEL_SIZE 8192

// Tree last level
#define TREE_LAST_LEVEL 13
// First virtual level
#define FIRST_VIRTUAL_LEVEL 14
// Leaf level
#define LEAF_LEVEL 20

// per level offset
static const uint32_t OCBT_depth_offset[21] = { 0, // Level 0
                                        32 * 1, // level 1
                                        32 * 1 + 32 * 2, // level 2
                                        32 * 1 + 32 * 2 + 32 * 4, // level 3
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8, // Level 4
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16, // Level 5
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32, // Level 6
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32 + 32 * 64, // Level 7

                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32 + 32 * 64 + 16 * 128, // Level 8
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32 + 32 * 64 + 16 * 128 + 16 * 256, // Level 9
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32 + 32 * 64 + 16 * 128 + 16 * 256 + 16 * 512, // Level 10
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32 + 32 * 64 + 16 * 128 + 16 * 256 + 16 * 512 + 16 * 1024, // Level 11
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32 + 32 * 64 + 16 * 128 + 16 * 256 + 16 * 512 + 16 * 1024 + 16 * 2048, // Level 12
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32 + 32 * 64 + 16 * 128 + 16 * 256 + 16 * 512 + 16 * 1024 + 16 * 2048 + 16 * 4096, // Level 13

                                        0, // Level 14
                                        0, // Level 15
                                        0, // Level 16
                                        0, // Level 17
                                        0, // Level 18
                                        0, // Level 19
                                        0, // Level 20
};

static const uint64_t OCBT_bit_mask[21] = { 0xffffffff, // Root 17
                            0xffffffff, // Level 16
                            0xffffffff, // level 15
                            0xffffffff, // level 14
                            0xffffffff, // level 13
                            0xffffffff, // level 12
                            0xffffffff, // level 11

                            0xffff, // level 10
                            0xffff, // level 9
                            0xffff, // level 8
                            0xffff, // level 8
                            0xffff, // level 8
                            0xffff, // level 8
                            0xff, // level 8

                            0xffffffffffffffff, // level 7
                            0xffffffff, // Level 6
                            0xffff, // level 5
                            0xff, // level 4
                            0xf, // level 3
                            0x3, // level 2
                            0x1, // level 1
};

static const uint32_t OCBT_bit_count[21] = { 32, // Root 17
                            32, // Level 16
                            32, // level 15
                            32, // level 14
                            32, // level 13
                            32, // level 12
                            32, // level 11

                            16, // level 10
                            16, // level 9
                            16, // level 8
                            16, // level 8
                            16, // level 8
                            16, // level 8
                            8, // level 8

                            64, // Level 5
                            32, // Level 5
                            16, // Level 4
                            8, // level 3
                            4, // level 2
                            2, // level 1
                            1, // level 0
};

// Implementation of the class
#define OCBT_CLASS UPCBT_1M
#define OCBT_NUM_ELEMENTS OCBT_1M_NUM_ELEMENTS
#include "cbt/OCBT_generic.h"