/*
 *      luma_bit.h
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *      HW4 Arith
 *
 *      - Header file declaring client-accessible functions for the
 *        luma_bit component
 *      - Component converts luminance values of pixels in a 2x2 block between
 *        uncompressed floating point representations in XYZ color space and
 *        compressed bit representations in DCT space
 */

#ifndef LUMABIT_INCLUDED
#define LUMABIT_INCLUDED

#include "pixelblock.h"

/* -- CONVERSION FUNCTIONS -- */
/*
 * Overwrites old luma values in bit with conversions from xyz, and returns bit
 * CRE: parameters cannot be NULL
 */
extern bit_block luma_to_bit(XYZ_block xyz, bit_block bit);

/*
 * Overwrites old luma values in xyz with conversions from bit, and returns xyz
 * CRE: parameters cannot be NULL
 */
extern XYZ_block bit_to_luma(bit_block bit, XYZ_block xyz);
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^ */

#endif /* LUMABIT_INCLUDED */