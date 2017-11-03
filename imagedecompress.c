/*
 *      imagedecompress40.c
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *      HW4 Arith
 *
 *      - File that defines the decompress methods for ImageMethods
 *      - Decompress: Reads a compressed COMP40 format image from the input
 *                    stream and decompresses it into an uncompressed portable
 *                    pixmap on standard output
 *      - Invariants:
 *              ~ Original image is never modified
 *              ~ Num of 2x2 blocks in img = (img_width / 2) * (img_height / 2)
 */

#include <stdio.h>
#include <stdlib.h>

#include "a2methods.h"
#include "a2plain.h"
#include "assert.h"
#include "chroma_bit.h"
#include "compress40.h"
#include "imagemethods.h"
#include "luma_bit.h"
#include "mem.h"
#include "pixpack.h"
#include "rgb_xyz.h"
#include "uarray.h"

/* -- struct Pnm_ppm is from pnm.h -- */
typedef struct Pnm_ppm    *ppm;

/* -- A2Methods_UArray2 is from a2methods.h -- */
typedef A2Methods_UArray2 A2;

/* -- GET PPM HELPER FUNCTIONS -- */
ppm            get_ppm    (unsigned width, unsigned height, A2Methods_T m);
struct Pnm_rgb get_pnm_rgb(RGB_px rgb);

/* -- FREE HELPER FUNCTIONS -- */
void free_rgb_d      (UArray_T rgb_blocks);
void free_xyz_d      (UArray_T xyz_blocks);
void free_bit_d      (UArray_T bit_blocks);
void free_codewords_d(UArray_T codewords);

/*
 * [Name]:       new_blocks
 * [Parameters]: 2 unsigned integers (length and size of UArray_T)
 * [Return]:     New UArray_T of length and size
 * [Purpose]:    Creates a new UArray_T with the given dimensions
 * [Errors]:     CRE is thrown from UArray if length or size are invalid
 */
static UArray_T new_blocks(unsigned length, unsigned size)
{
        return UArray_new(length, size);
}

/* -------------------------------------------- *
 *                 XYZ / RGB                    |
 * v------------------------------------------v */
/*
 * [Name]:       rgb_xyz
 * [Parameters]: 2 UArrays (<output>: rgb_blocks, <input>: xyz_blocks)
 * [Return]:     rgb_blocks that have pixel values in RGB color space
 * [Purpose]:    Converts pixel values from XYZ color space in xyz_blocks to
 *               RGB color space in rgb_blocks
 * [Errors]:     CRE if any parameter is NULL or not malloc'd
 */
static UArray_T rgb_xyz(UArray_T rgb_blocks, UArray_T xyz_blocks)
{
        assert(xyz_blocks != NULL && rgb_blocks != NULL);

        for (int i = 0; i < UArray_length(xyz_blocks); i++) {
                XYZ_block xyz = UArray_at(xyz_blocks, i);
                RGB_block rgb = UArray_at(rgb_blocks, i);

                rgb = XYZ_to_RGB(xyz, rgb);
        }

        return rgb_blocks;
}
/* ^------------------------------------------^ */

/* -------------------------------------------- *
 *                  CHROMA                     |
 * v------------------------------------------v */
/*
 * [Name]:       chroma
 * [Parameters]: 2 UArrays (<output>: xyz_blocks, <input>: bit_blocks)
 * [Return]:     xyz_blocks that have chroma values converted from their bit
 *               representations to XYZ color space, and luma values unchanged
 * [Purpose]:    Converts pixel chroma values from their bit representations in
 *               bit_blocks to XYZ color space in xyz_blocks
 * [Errors]:     CRE if any parameter is NULL or not malloc'd
 */
static UArray_T chroma(UArray_T xyz_blocks, UArray_T bit_blocks)
{
        assert(bit_blocks != NULL && xyz_blocks != NULL);

        for (int i = 0; i < UArray_length(bit_blocks); i++) {
                XYZ_block xyz = UArray_at(xyz_blocks, i);
                bit_block bit = UArray_at(bit_blocks, i);

                xyz = bit_to_chroma(bit, xyz);
        }

        return xyz_blocks;
}
/* ^------------------------------------------^ */

/* -------------------------------------------- *
 *                   LUMA                       |
 * v------------------------------------------v */
/*
 * [Name]:       luma
 * [Parameters]: 2 UArrays (<output>: xyz_blocks, <input>: bit_blocks)
 * [Return]:     xyz_blocks that have luma values converted from their bit
 *               representations to XYZ color space, and chroma values unchanged
 * [Purpose]:    Converts pixel luma values from their bit representations
 *               (DCT space) in bit_blocks to XYZ color space in xyz_blocks
 * [Errors]:     CRE if any parameter is NULL or not malloc'd
 */
static UArray_T luma(UArray_T xyz_blocks, UArray_T bit_blocks)
{
        assert(bit_blocks != NULL && xyz_blocks != NULL);

        for (int i = 0; i < UArray_length(bit_blocks); i++) {
                XYZ_block xyz = UArray_at(xyz_blocks, i);
                bit_block bit = UArray_at(bit_blocks, i);

                xyz = bit_to_luma(bit, xyz);
        }

        return xyz_blocks;
}
/* ^------------------------------------------^ */

/* -------------------------------------------- *
 *                  PIXPACK                     |
 * v------------------------------------------v */
/*
 * [Name]:       pixpack
 * [Parameters]: 2 UArrays (<output>: bit_blocks, <input>: codewords)
 * [Return]:     A UArray of bit_blocks, where each bit_block has the values
 *               unpacked from a 32-bit codeword
 * [Purpose]:    Unpacks 32-bit codewords into individual bit fields in
 *               bit_blocks
 * [Errors]:     CRE if any parameter is NULL or not malloc'd
 */
static UArray_T pixpack(UArray_T bit_blocks, UArray_T codewords)
{
        assert(codewords != NULL && bit_blocks != NULL);

        for (int i = 0; i < UArray_length(codewords); i++) {
                bit_block buf = UArray_at(bit_blocks, i);
                buf = unpack(*((uint32_t *)UArray_at(codewords, i)), buf);
        }

        return bit_blocks;
}
/* ^------------------------------------------^ */

/* -------------------------------------------- *
 *                   WRITE                      |
 * v------------------------------------------v */
/*
 * [Name]:       write
 * [Parameters]: 1 UArray (rgb_blocks), 2 unsigned integers (width, height)
 * [Return]:     void
 * [Purpose]:    Prints decompressed image stored in rgb_blocks to standard
 *               output in a portable pixmap format (stored in row-major order)
 * [Errors]:     CRE if rgb_blocks is NULL
 */
static void write(UArray_T rgb_blocks, unsigned width, unsigned height)
{
        assert(rgb_blocks != NULL);

        A2Methods_T m = uarray2_methods_plain;
        A2 pixels     = m->new(width, height, sizeof(struct Pnm_rgb));
        ppm image     = get_ppm(width, height, m);

        int cell = 0;
        for (unsigned row = 0; row < height; row += 2) {
                for (unsigned col = 0; col < width; col += 2) {
                        RGB_block block = UArray_at(rgb_blocks, cell);

                        Pnm_rgb buf = (Pnm_rgb)m->at(pixels, col, row);
                               *buf = get_pnm_rgb(block->topL);

                                buf = (Pnm_rgb)m->at(pixels, col + 1, row);
                               *buf = get_pnm_rgb(block->topR);

                                buf = (Pnm_rgb)m->at(pixels, col, row + 1);
                               *buf = get_pnm_rgb(block->botL);

                                buf = (Pnm_rgb)m->at(pixels, col + 1, row + 1);
                               *buf = get_pnm_rgb(block->botR);

                        cell++;
                }
        }

        image->pixels = pixels;
        Pnm_ppmwrite(stdout, image);
        Pnm_ppmfree(&image);
}

/*
 * [Name]:       get_ppm
 * [Parameters]: 2 unsigned integers (width, height), 1 A2Methods_T
 * [Return]:     Pnm_ppm object with metadata initialized, but no image
 * [Purpose]:    Initializes a ppm with the given metadata
 * [Errors]:     CRE if methods is NULL
 */
ppm get_ppm(unsigned width, unsigned height, A2Methods_T m)
{
        assert(m != NULL);

        ppm pixmap;
        NEW(pixmap);

        pixmap->width       = width;
        pixmap->height      = height;
        pixmap->denominator = RGB_MAX;
        pixmap->methods     = m;

        return pixmap;
}

/*
 * [Name]:       get_pnm_rgb
 * [Parameters]: 1 RGB_px
 * [Return]:     Pnm_rgb with same RGB values as stored in RGB_px
 * [Purpose]:    Initializes a Pnm_rgb pixel with the same values as its
 *               equivalent RGB_px
 * [Errors]:     CRE if RGB_px is NULL
 */
struct Pnm_rgb get_pnm_rgb(RGB_px rgb)
{
        assert(rgb != NULL);

        struct Pnm_rgb pnm = { .red = 0, .green = 0, .blue = 0 };
        pnm.red   = rgb->r;
        pnm.green = rgb->g;
        pnm.blue  = rgb->b;

        return pnm;
}
/* ^------------------------------------------^ */

/* -------------------------------------------- *
 *                  FREE                        |
 * v------------------------------------------v */
/*
 * [Name]:       free_d
 * [Parameters]: 4 UArrays, 1 ppm
 * [Return]:     void
 * [Purpose]:    Frees the given UArrays (ppm will always be passed as NULL)
 * [Errors]:     CRE if image is NOT NULL
 */
static void free_d(UArray_T rgb_blocks, UArray_T xyz_blocks, UArray_T bit_blocks,
                   UArray_T codewords,  ppm image)
{
        assert(image == NULL);

        free_rgb_d      (rgb_blocks);
        free_xyz_d      (xyz_blocks);
        free_bit_d      (bit_blocks);
        free_codewords_d(codewords);
}

/*
 * [Name]:       free_rgb_d
 * [Parameters]: 1 UArray (rgb_blocks)
 * [Return]:     void
 * [Purpose]:    Frees the given UArray of rgb_blocks and the pixels within
 *               each RGB_block
 * [Errors]:     None
 */
void free_rgb_d(UArray_T rgb_blocks)
{
        for (int i = 0; i < UArray_length(rgb_blocks); i++) {
                RGB_block block = UArray_at(rgb_blocks, i);
                free_RGB_block(block);
        }

        UArray_free(&rgb_blocks);
}

/*
 * [Name]:       free_xyz_d
 * [Parameters]: 1 UArray (xyz_blocks)
 * [Return]:     void
 * [Purpose]:    Frees the given UArray of xyz_blocks and the pixels within
 *               each XYZ_block
 * [Errors]:     None
 */
void free_xyz_d(UArray_T xyz_blocks)
{
        for (int i = 0; i < UArray_length(xyz_blocks); i++) {
                XYZ_block block = UArray_at(xyz_blocks, i);
                free_XYZ_block(block);
        }

        UArray_free(&xyz_blocks);
}

/*
 * [Name]:       free_bit_d
 * [Parameters]: 1 UArray (bit_blocks)
 * [Return]:     void
 * [Purpose]:    Frees the given UArray of bit_blocks
 * [Errors]:     None
 */
void free_bit_d(UArray_T bit_blocks)
{
        UArray_free(&bit_blocks);
}

/*
 * [Name]:       free_codewords_d
 * [Parameters]: 1 UArray (codewords)
 * [Return]:     void
 * [Purpose]:    Frees the given UArray of codewords
 * [Errors]:     None
 */
void free_codewords_d(UArray_T codewords)
{
        UArray_free(&codewords);
}
/* ^------------------------------------------^ */

/* Private struct with function pointers */
static struct ImageMethods_T decompress_struct = {
        new_blocks,
        NULL, /* read */
        rgb_xyz,
        chroma,
        luma,
        pixpack,
        write,
        free_d,
};

/* The Payoff: Exported pointer to the struct */
ImageMethods_T decompress = &decompress_struct;
/* -------------------------------------------- */