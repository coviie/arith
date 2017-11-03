/*
 *      chroma_bit.h
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *
 *      - Header file declaring client-accessible functions for the
 *        chroma_bit component
 *      - Component converts chroma values of pixels in a 2x2 block between
 *        uncompressed floating point representations and compressed bit
 *        representations
 */

#ifndef CHROMABIT_INCLUDED
#define CHROMABIT_INCLUDED

#include "pixelblock.h"

/* -- CONVERSION FUNCTIONS -- */
/*
 * Overwrites chroma values in bit with conversions from xyz, and returns bit
 * CRE: parameters cannot be NULL
 */
extern bit_block chroma_to_bit(XYZ_block xyz, bit_block bit);

/*
 * Overwrites chroma values in xyz with conversions from bit, and returns xyz
 * CRE: parameters cannot be NULL
 */
extern XYZ_block bit_to_chroma(bit_block bit, XYZ_block xyz);
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^ */

#endif /* CHROMA_INCLUDED */