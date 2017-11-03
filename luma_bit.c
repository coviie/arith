/*
 *      luma_bit.c
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *      HW4 Arith
 *
 *      - Component file defining all extern and helper functions for the
 *        luma_bit component
 *      - Component converts luminance values of pixels in a 2x2 block between
 *        uncompressed floating point representations in XYZ color space and
 *        compressed bit representations in DCT space
 *      - Component-wide invariants:
 *              ~ a range is [0, 1]
 *              ~ {b, c, d} range is [-0.3, 0.3]
 *              ~ Blocks passed in as "input" are not modified
 *              ~ Quantized a has the range of [0, 2^A_WIDTH - 1]
 *              ~ Quantized b has the range of [-(2^(B_WIDTH - 1) - 1),
 *                                              (2^(B_WIDTH - 1) - 1)]
 *              ~ Quantized c and d have a similar range to b (except with
 *                C_WIDTH and D_WIDTH respectively)
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "assert.h"
#include "mem.h"
#include "luma_bit.h"

/* -- Struct to hold a block's luma values in DCT space -- */
typedef struct cosine {
        float a, b, c, d;
} cosine;
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* -- Range of values for cosine coefficients -- */
const float A_MAX = 1;
const float A_MIN = 0;
const float BCD_MAX = 0.3;
const float BCD_MIN = -0.3;
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* -- DISCRETE COSINE TRANSFORM (DCT) HELPER FUNCTIONS -- */
cosine    dct        (XYZ_block xyz);
XYZ_block inverse_dct(XYZ_block xyz, cosine luma_cosine);
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* -- QUANTIZATION HELPER FUNCTIONS -- */
unsigned quantize_a  (float value, int bitsize);
int      quantize_bcd(float value, int bitsize);
float    scale_a     (float a, int bitsize);
float    scale_bcd   (float value, int bitsize);
float    fit_range   (float value, float max, float min);
uint64_t get_max     (int bitsize);
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/*---------------------------------------------------------------
 |                 COMPRESS CONVERSION FUNCTIONS                |
 *--------------------------------------------------------------*/
/*
 * [Name]:       luma_to_bit
 * [Parameters]: 1 XYZ_block, 1 bit_block
 *               Note: Overwrites existing luma values in bit
 * [Return]:     bit_block, with only luma values overwritten with values
 *               converted from XYZ_block
 * [Purpose]:    Converts luma values in 2x2 block from floating point in XYZ
 *               color space to bit representations in DCT space
 *               Note: Does not modify values in xyz or chroma values in bit
 *                     Valid range for quantized DCT values are specified in
 *                      the constants defined at pixelblock.h
 * [Errors]:     CRE if any block is NULL or has not been malloc'd
 *               URE if client loses pointer to blocks
 */
bit_block luma_to_bit(XYZ_block xyz, bit_block bit)
{
        assert(xyz != NULL && bit != NULL);

        cosine luma_cosine = dct(xyz);

        bit->a = quantize_a  (luma_cosine.a, A_WIDTH);
        bit->b = quantize_bcd(luma_cosine.b, B_WIDTH);
        bit->c = quantize_bcd(luma_cosine.c, C_WIDTH);
        bit->d = quantize_bcd(luma_cosine.d, D_WIDTH);

        return bit;
}

/*
 * [Name]:       dct
 * [Parameters]: 1 XYZ_block
 * [Return]:     cosine, containing luma values in floating point in DCT space
 * [Purpose]:    Converts luma values in 2x2 block from floating point in XYZ
 *               color space to floating point in DCT space
 *               Note: Does not modify values in xyz
 * [Errors]:     CRE if any pixel/the block is NULL or has not been malloc'd
 *               URE if client loses pointer to block
 */
cosine dct(XYZ_block xyz)
{
        assert(xyz != NULL && xyz->topL != NULL && xyz->topR != NULL &&
               xyz->botL != NULL && xyz->botR != NULL);

        cosine luma_cosine;

        float y1 = xyz->topL->luma;
        float y2 = xyz->topR->luma;
        float y3 = xyz->botL->luma;
        float y4 = xyz->botR->luma;

        luma_cosine.a = (y4 + y3 + y2 + y1) / 4.0;
        luma_cosine.b = (y4 + y3 - y2 - y1) / 4.0;
        luma_cosine.c = (y4 - y3 + y2 - y1) / 4.0;
        luma_cosine.d = (y4 - y3 - y2 + y1) / 4.0;

        return luma_cosine;
}

/*
 * [Name]:       quantize_a
 * [Parameters]: 1 float (a), 1 int (size of bitfield that a codes to)
 * [Return]:     bit representation of a in an unsigned scaled integer
 * [Purpose]:    Converts a from its floating point representation to its
 *               scaled/quantized bit representation
 *               Note: Range of quantized a value is [0, max]
 *                     Quantized a is guaranteed to fit in bitsize no. of bits
 * [Errors]:     URE if bitsize > 63 (note: unlikely in our use)
 */
unsigned quantize_a(float a, int bitsize)
{
        uint64_t max = get_max(bitsize);
        a = fit_range(a, A_MAX, A_MIN);

        return a * (max / A_MAX);
}

/*
 * [Name]:       quantize_bcd
 * [Parameters]: 1 float (value), 1 int (size of bitfield that value codes to)
 * [Return]:     bit representation of value in an signed scaled integer
 * [Purpose]:    Converts value from its floating point representation to its
 *               scaled/quantized bit representation
 *               Note: Range of quantized value is [-max, max]
 *                     Quantized value guaranteed to fit in bitsize no. of bits
 * [Errors]:     URE if bitsize > 64 (note: unlikely in our use)
 */
int quantize_bcd(float value, int bitsize)
{
        uint64_t max = get_max(bitsize - 1);
        value = fit_range(value, BCD_MAX, BCD_MIN);

        return value * (max / BCD_MAX);
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/*---------------------------------------------------------------
 |                DECOMPRESS CONVERSION FUNCTIONS               |
 *--------------------------------------------------------------*/
/*
 * [Name]:       bit_to_luma
 * [Parameters]: 1 XYZ_block, 1 bit_block
 *               Note: Overwrites existing luma values in xyz
 * [Return]:     xyz_block, with only luma values overwritten with values
 *               converted from XYZ_block
 * [Purpose]:    Converts luma values in 2x2 block from bit representations in
 *               DCT space to floating point in XYZ color space
 *               Note: Does not modify values in bit or chroma values in xyz
 *                     Range for luma values in xyz is [0, 1]
 * [Errors]:     CRE if any block is NULL or has not been malloc'd
 *               URE if client loses pointer to blocks
 */
XYZ_block bit_to_luma(bit_block bit, XYZ_block xyz)
{
        assert(bit != NULL && xyz != NULL);

        cosine luma_cosine;
        luma_cosine.a = scale_a  (bit->a, A_WIDTH);
        luma_cosine.b = scale_bcd(bit->b, B_WIDTH);
        luma_cosine.c = scale_bcd(bit->c, C_WIDTH);
        luma_cosine.d = scale_bcd(bit->d, D_WIDTH);

        return inverse_dct(xyz, luma_cosine);
}

/*
 * [Name]:       scale_a
 * [Parameters]: 1 unsigned scaled int (a), 1 int (width of a in bits)
 * [Return]:     floating point representation of a
 * [Purpose]:    Converts a from its scaled/quantized bit representation to
 *               its floating point representation
 *               Note: Range of floating point a value is [A_MIN, A_MAX]
 * [Errors]:     URE if bitsize > 63 (note: unlikely in our use)
 */
float scale_a(float a, int bitsize)
{
        uint64_t max = get_max(bitsize);
        a = a / (max / A_MAX);

        return fit_range(a, A_MAX, A_MIN);
}

/*
 * [Name]:       scale_bcd
 * [Parameters]: 1 signed scaled int (value), 1 int (width of value in bits)
 * [Return]:     floating point representation of value
 * [Purpose]:    Converts value from its scaled/quantized bit representation to
 *               its floating point representation
 *               Note: Range of floating point value is [BCD_MIN, BCD_MAX]
 * [Errors]:     URE if bitsize > 64 (note: unlikely in our use)
 */
float scale_bcd(float value, int bitsize)
{
        uint64_t max = get_max(bitsize - 1);
        value = value / (max / BCD_MAX);

        return fit_range(value, BCD_MAX, BCD_MIN);
}

/*
 * [Name]:       inverse_dct
 * [Parameters]: 1 XYZ_block, 1 cosine struct
 * [Return]:     xyz, containing luma values in floating point in XYZ space
 * [Purpose]:    Converts luma values in 2x2 block from floating point in DCT
 *               space to floating point in XYZ color space
 * [Errors]:     CRE if the block is NULL or has not been malloc'd
 *               URE if client loses pointer to block or if pixels have been
 *                   malloc'd previously
 */
XYZ_block inverse_dct(XYZ_block xyz, cosine luma_cosine)
{
        assert(xyz != NULL);

        NEW(xyz->topL);
        NEW(xyz->topR);
        NEW(xyz->botL);
        NEW(xyz->botR);

        float a = luma_cosine.a;
        float b = luma_cosine.b;
        float c = luma_cosine.c;
        float d = luma_cosine.d;

        xyz->topL->luma = (a - b - c + d);
        xyz->topR->luma = (a - b + c - d);
        xyz->botL->luma = (a + b - c - d);
        xyz->botR->luma = (a + b + c + d);

        return xyz;
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/*---------------------------------------------------------------
 |                 QUANTIZATION HELPER FUNCTIONS                |
 *--------------------------------------------------------------*/
/*
 * [Name]:       fit_range
 * [Parameters]: 3 floats (value, max, min)
 * [Return]:     Value that fits within the range [min, max]
 * [Purpose]:    Force value to fit within the range [min, max]
 * [Errors]:     None
 */
float fit_range(float value, float max, float min)
{
        if (value > max) {
                return max;
        } else if (value < min) {
                return min;
        } else {
                return value;
        }
}

/*
 * [Name]:       get_max
 * [Parameters]: 1 int (bitsize)
 * [Return]:     Largest integer that can fit within a bitfield of bitsize
 * [Purpose]:    Helper function to do a bitshift, to obtain maximum integer
 *               that can fit within bitsize num. of bits
 * [Errors]:     URE if bitsize > 63 (note: unlikely in our use)
 */
uint64_t get_max(int bitsize)
{
        return (((uint64_t)1 << bitsize) - 1);
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */