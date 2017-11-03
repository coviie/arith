/*
 *      chroma_bit.c
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *
 *      - Component file declaring all extern and helper functions for the
 *        chroma_bit component
 *      - Component converts chroma values of pixels in a 2x2 block between
 *        uncompressed floating point representations and compressed bit
 *        representations
 *      - Component-wide invariants:
 *              ~ {Pb, Pr} range is [-0.5, 0.5]
 *              ~ Blocks passed in as "input" are not modified
 *              ~ {Index Pb, Index Pr} range is [0, 15]
 */

#include <stdio.h>
#include <stdlib.h>

#include "arith40.h"
#include "assert.h"
#include "mem.h"
#include "chroma_bit.h"

/* -- COMPRESSION AVERAGE HELPER FUNCTIONS -- */
float average_Pb(XYZ_block xyz);
float average_Pr(XYZ_block xyz);
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* -- DECOPMRESSION HELPER FUNCTIONS -- */
XYZ_block store_Pb(XYZ_block xyz, float Pb);
XYZ_block store_Pr(XYZ_block xyz, float Pr);
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/*--------------------------------------------------------------*
 |                 COMPRESS CONVERSION FUNCTIONS                |
 *--------------------------------------------------------------*/
/*
 * [Name]:       chroma_to_bit
 * [Parameters]: 1 XYZ_block, 1 bit_block
 *               Note: Overwrites existing chroma values in bit
 * [Return]:     bit_block, with only chroma values overwritten with values
 *               converted from XYZ_block
 * [Purpose]:    Converts chroma values in 2x2 block from floating point to bit
 *               representations
 *               Note: Does not modify values in xyz or luma values in bit
 * [Errors]:     CRE if any block is NULL or has not been malloc'd
 *               URE if client loses pointer to blocks
 */
bit_block chroma_to_bit(XYZ_block xyz, bit_block bit)
{
        assert(xyz != NULL && bit != NULL);

        float Pb = average_Pb(xyz);
        float Pr = average_Pr(xyz);

        bit->Pb = Arith40_index_of_chroma(Pb);
        bit->Pr = Arith40_index_of_chroma(Pr);

        return bit;
}

/*
 * [Name]:       average_Pb
 * [Parameters]: 1 XYZ_block
 * [Return]:     Average Pb value for all pixels in xyz, in a float
 * [Purpose]:    Calculates the average Pb value for the pixels in the block
 *               Note: Does not modify values in xyz
 * [Errors]:     CRE if the block/any pixel is NULL or has not been malloc'd
 */
float average_Pb(XYZ_block xyz)
{
        assert(xyz != NULL);

        float Pb = 0.0;

        Pb += xyz->topL->Pb;
        Pb += xyz->topR->Pb;
        Pb += xyz->botL->Pb;
        Pb += xyz->botR->Pb;
        Pb /= 4.0;

        return Pb;
}

/*
 * [Name]:       average_Pr
 * [Parameters]: 1 XYZ_block
 * [Return]:     Average Pr value for all pixels in xyz, in a float
 * [Purpose]:    Calculates the average Pr value for the pixels in the block
 *               Note: Does not modify values in xyz
 * [Errors]:     CRE if the block/any pixel is NULL or has not been malloc'd
 */
float average_Pr(XYZ_block xyz)
{
        assert(xyz != NULL);

        float Pr = 0.0;

        Pr += xyz->topL->Pr;
        Pr += xyz->topR->Pr;
        Pr += xyz->botL->Pr;
        Pr += xyz->botR->Pr;
        Pr /= 4.0;

        return Pr;
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/*--------------------------------------------------------------*
 |                DECOMPRESS CONVERSION FUNCTIONS               |
 *--------------------------------------------------------------*/
/*
 * [Name]:       chroma_to_bit
 * [Parameters]: 1 XYZ_block, 1 bit_block
 *               Note: Overwrites existing chroma values in xyz
 * [Return]:     xyz_block, with only chroma values overwritten with values
 *               converted from bit_block
 * [Purpose]:    Converts chroma values in 2x2 block from bit representations
 *               to floating point numbers
 *               Note: Does not modify values in bit or luma values in xyz
 * [Errors]:     CRE if any block is NULL or has not been malloc'd
 *               URE if client loses pointer to blocks
 */
XYZ_block bit_to_chroma(bit_block bit, XYZ_block xyz)
{
        assert(bit != NULL && xyz != NULL);

        float Pb = Arith40_chroma_of_index(bit->Pb);
        float Pr = Arith40_chroma_of_index(bit->Pr);

        xyz = store_Pb(xyz, Pb);
        xyz = store_Pr(xyz, Pr);
        return xyz;
}

/*
 * [Name]:       store_Pb
 * [Parameters]: 1 XYZ_block, 1 float(Pb)
 *               Note: Overwrites existing Pb values in xyz
 * [Return]:     xyz_block, with only Pb values overwritten
 * [Purpose]:    Replace existing Pb value with the average of all pixels in
 *               2x2 block
 *               Note: Does not modify luma or Pr values in xyz
 * [Errors]:     CRE if block is NULL or has not been malloc'd
 *               URE if client loses pointer to blocks
 */
XYZ_block store_Pb(XYZ_block xyz, float Pb)
{
        assert(xyz != NULL);

        xyz->topL->Pb = Pb;
        xyz->topR->Pb = Pb;
        xyz->botL->Pb = Pb;
        xyz->botR->Pb = Pb;

        return xyz;
}

/*
 * [Name]:       store_Pr
 * [Parameters]: 1 XYZ_block, 1 float(Pr)
 *               Note: Overwrites existing Pr values in xyz
 * [Return]:     xyz_block, with only Pr values overwritten
 * [Purpose]:    Replace existing Pr value with the average of all pixels in
 *               2x2 block
 *               Note: Does not modify luma or Pb values in xyz
 * [Errors]:     CRE if block is NULL or has not been malloc'd
 *               URE if client loses pointer to blocks
 */
XYZ_block store_Pr(XYZ_block xyz, float Pr)
{
        assert(xyz != NULL);

        xyz->topL->Pr = Pr;
        xyz->topR->Pr = Pr;
        xyz->botL->Pr = Pr;
        xyz->botR->Pr = Pr;

        return xyz;
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */