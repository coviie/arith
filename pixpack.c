/*
 *      pixpack.h
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *
 *      - Component file declaring all extern and helper functions for the
 *        pixpack component
 *      - Component packs compressed bit representations of pixel values
 *        in a 2x2 block into a 32-bit codeword, such that each 32-bit codeword
 *        represents 1 2x2 pixel block
 *      - Component accesses a 32-bit codeword by bytes, allowing for byte
 *        extraction, and byte storing within a codeword
 *      - Component-wide invariants:
 *              ~ Bit blocks passed in as "input" are not modified
 *              ~ There are BITS_IN_BYTE bits in a byte
 *              ~ Widths of each bitfield are specified in pixelblock.h
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "assert.h"
#include "bitpack.h"
#include "mem.h"
#include "pixpack.h"

/*---------------------------------------------------------------
 |                 COMPRESS CONVERSION FUNCTIONS                |
 *--------------------------------------------------------------*/
/*
 * [Name]:       pack
 * [Parameters]: 1 bit_block
 * [Return]:     codeword filled with bitfields from bit_block, stored in a
 *               32-bit integer
 * [Purpose]:    Packs all bitfields in bit into 32-bit codeword, according
 *               to a certain order
 *               Note: Does not modify values in bit
 * [Errors]:     CRE if block is NULL or has not been malloc'd, or if final
 *                   codeword does not have 32-bits
 */
uint32_t pack(bit_block bit)
{
        assert(bit != NULL);

        unsigned lsb      = 0;
        uint32_t codeword = 0;

        codeword = Bitpack_newu(codeword, PR_WIDTH, lsb, bit->Pr);
        lsb += PR_WIDTH;

        codeword = Bitpack_newu(codeword, PB_WIDTH, lsb, bit->Pb);
        lsb += PB_WIDTH;

        codeword = Bitpack_news(codeword, D_WIDTH, lsb, bit->d);
        lsb += D_WIDTH;

        codeword = Bitpack_news(codeword, C_WIDTH, lsb, bit->c);
        lsb += C_WIDTH;

        codeword = Bitpack_news(codeword, B_WIDTH, lsb, bit->b);
        lsb += B_WIDTH;

        codeword = Bitpack_newu(codeword, A_WIDTH, lsb, bit->a);
        lsb += A_WIDTH;

        assert(lsb == BITS_IN_BYTE * sizeof(codeword));

        return codeword;
}

/*
 * [Name]:       extract_char
 * [Parameters]: 1 uint32_t (codeword), 1 int (byte index in codeword)
 * [Return]:     index_th byte extracted from codeword, stored in a char
 * [Purpose]:    Extracts (but doesn't remove) index_th byte stored in codeword
 *               for use with putchar
 *               Note: Does not modify values in codeword
 * [Errors]:     CREs will come from Bitpack component
 */
char extract_char(uint32_t codeword, int index)
{
        return Bitpack_getu(codeword, BITS_IN_BYTE * sizeof(char),
                            index * BITS_IN_BYTE * sizeof(char));
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/*---------------------------------------------------------------
 |                DECOMPRESS CONVERSION FUNCTIONS               |
 *--------------------------------------------------------------*/
/*
 * [Name]:       unpack
 * [Parameters]: 1 uint32_t (codeword), 1 bit_block
 * [Return]:     bit filled with bitfields from codeword
 * [Purpose]:    Unpacks 32-bit codeword into the bitfields in bit, according
 *               to a certain order
 *               Note: Does not modify values in codeword
 * [Errors]:     CRE if block is NULL or has not been malloc'd, or if final
 *                   block does not have all its bitfields filled
 */
bit_block unpack(uint32_t codeword, bit_block bit)
{
        assert(bit != NULL);

        unsigned lsb = 0;

        bit->Pr = Bitpack_getu(codeword, PR_WIDTH, lsb);
        lsb += PR_WIDTH;

        bit->Pb = Bitpack_getu(codeword, PB_WIDTH, lsb);
        lsb += PB_WIDTH;

        bit->d = Bitpack_gets(codeword, D_WIDTH, lsb);
        lsb += D_WIDTH;

        bit->c = Bitpack_gets(codeword, C_WIDTH, lsb);
        lsb += C_WIDTH;

        bit->b = Bitpack_gets(codeword, B_WIDTH, lsb);
        lsb += B_WIDTH;

        bit->a = Bitpack_getu(codeword, A_WIDTH, lsb);
        lsb += A_WIDTH;

        assert(lsb == BITS_IN_BYTE * sizeof(codeword));

        return bit;
}

/*
 * [Name]:       store_char
 * [Parameters]: 1 char (c), 1 uint32_t (codeword), 1 int (byte index)
 * [Return]:     codeword with index_th byte replaced by bit contents of c
 * [Purpose]:    Replaces index_th byte stored in codeword with the bit
 *               contents of c instead
 * [Errors]:     CREs will come from Bitpack component
 */
uint32_t store_char(char c, uint32_t codeword, int index)
{
        uint64_t char_bit = Bitpack_getu(c, BITS_IN_BYTE * sizeof(char), 0);

        return Bitpack_newu(codeword, BITS_IN_BYTE * sizeof(char),
                            index * BITS_IN_BYTE * sizeof(char), char_bit);
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */