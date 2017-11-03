/*
 *      pixpack.h
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *      HW4 Arith
 *
 *      - Header file declaring client-accessible functions for the
 *        pixpack component
 *      - Component packs compressed bit representations of pixel values
 *        in a 2x2 block into a 32-bit codeword, such that each 32-bit codeword
 *        represents 1 2x2 pixel block
 *      - Component accesses a 32-bit codeword by bytes, allowing for byte
 *        extraction, and byte storing within a codeword
 */

#ifndef PIXPACK_INCLUDED
#define PIXPACK_INCLUDED

#include "pixelblock.h"

/* -- CONVERSION FUNCTIONS -- */
/*
 * Packs all bitfields in bit into a 32-bit codeword stored in an integer
 * CRE: parameter cannot be NULL
 */
extern uint32_t pack(bit_block bit);

/*
 * Unpacks a 32-bit codeword stored in an integer into the bitfields in bit
 * CRE: parameters cannot be NULL
 */
extern bit_block unpack(uint32_t codeword, bit_block bit);
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* -- BYTE-ACCESS FUNCTIONS -- */
/*
 * Extracts the index_th byte from the codeword into a char
 * CRE: index * BITS_IN_BYTE cannot exceed 32
 */
extern char extract_char(uint32_t codeword, int index);

/*
 * Stores c as the index_th byte in the codeword, replacing the existing byte
 * CRE: index * BITS_IN_BYTE cannot exceed 32
 */
extern uint32_t store_char(char c, uint32_t codeword, int index);
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

#endif /* PIXPACK_INCLUDED */