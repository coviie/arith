/*
 *      imagecompress40.c
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *      HW4 Arith
 *
 *      - File that defines the compress methods for ImageMethods
 *      - Compress: Reads an uncompressed portable pixmap from the input stream
 *                  and compresses it into a COMP40 format on standard output
 *      - Invariants:
 *              ~ Original image is never modified
 *              ~ Num of 2x2 blocks in img = (img_width / 2) * (img_height / 2)
 */

#include <stdio.h>
#include <stdlib.h>

#include "a2methods.h"
#include "a2blocked.h"
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
typedef struct Pnm_ppm          *ppm;

/* -- A2Methods definitions are from a2methods.h -- */
typedef A2Methods_UArray2       A2;
typedef A2Methods_Object        object;
typedef A2Methods_smallmapfun   smallmapfun;
typedef A2Methods_smallapplyfun smallapplyfun;

/* -- READ HELPER FUNCTIONS -- */
void     scale_ppm     (ppm image);
void     scale         (object *px, void *cl);
void     trim_ppm      (Pnm_ppm image);
UArray_T get_rgb_blocks(UArray_T rgb_blocks, ppm image);
RGB_px   get_rgb_pixel (Pnm_rgb pnm, float denom);

/* -- FREE HELPER FUNCTIONS -- */
void free_rgb_c      (UArray_T rgb_blocks);
void free_xyz_c      (UArray_T xyz_blocks);
void free_bit_c      (UArray_T bit_blocks);
void free_codewords_c(UArray_T codewords);

/* -------------------------------------------- *
 *         NEW BLOCK ALLOCATION FUNCTION        |
 * v------------------------------------------v */
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

/* -------------------------------------------- */


/* -------------------------------------------- *
 *                   READ                       |
 * v------------------------------------------v */
/*
 * [Name]:       read
 * [Parameters]: 1 UArray (rgb_blocks), 1 Pnm_ppm (image)
 * [Return]:     rgb_blocks filled with RGB values of the pixels in given image
 * [Purpose]:    Copies RGB values from pixmap in image into the UArray of 2x2
 *               RGB blocks, and trims image to even dimensions if necessary
 * [Errors]:     CRE if any parameter is NULL or not malloc'd
 */
static UArray_T read(UArray_T rgb_blocks, Pnm_ppm image)
{
        assert(rgb_blocks != NULL && image != NULL);

        scale_ppm(image);
        trim_ppm (image);
        rgb_blocks = get_rgb_blocks(rgb_blocks, image);

        return rgb_blocks;
}

/*
 * [Name]:       scale_ppm
 * [Parameters]: 1 Pnm_ppm (image)
 * [Return]:     void
 * [Purpose]:    Scales the given image based on the given denominator by
 *               calling a smallmap function on the pixmap
 * [Errors]:     CRE if image is NULL or not malloc'd
 */
void scale_ppm(ppm image)
{
        assert(image != NULL);

        smallmapfun   *map   = image->methods->small_map_default;
        smallapplyfun *apply = scale;

        map(image->pixels, apply, &(image->denominator));

        image->denominator = RGB_MAX;
}

/*
 * [Name]:       scale
 * [Parameters]: 1 object* (pixel in image pixmap), 1 void* (closure, contains
 *               the denominator of the image's pixel values)
 * [Return]:     void
 * [Purpose]:    Smallmap function that scales each px's RGB to range of
 *               [0, 255]
 * [Errors]:     None
 */
void scale(object *px, void *cl)
{
        int    *denominator = (int *) cl;
        Pnm_rgb pixel       = (Pnm_rgb) px;
        float   den_scale   = (float) *denominator / RGB_MAX;

        pixel->red   = ((float) pixel->red)   / den_scale;
        pixel->green = ((float) pixel->green) / den_scale;
        pixel->blue  = ((float) pixel->blue)  / den_scale;
}

/*
 * [Name]:       trim_ppm
 * [Parameters]: 1 Pnm_ppm (image)
 * [Return]:     void
 * [Purpose]:    Trims the given ppm to even dimensions
 * [Errors]:     CRE if image is NULL or not malloc'd
 */
void trim_ppm(ppm image)
{
        assert(image != NULL);

        if (image->width % 2 != 0) {
                (image->width)--;
        }

        if (image->height % 2 != 0) {
                (image->height)--;
        }
}

/*
 * [Name]:       get_rgb_blocks
 * [Parameters]: 1 UArray (rgb_blocks), 1 Pnm_ppm (image)
 * [Return]:     rgb_blocks filled with RGB values of the pixels in given image
 * [Purpose]:    Copies RGB values from pixmap in image into the UArray of 2x2
 *               RGB blocks
 * [Errors]:     CRE if any parameter is NULL or not malloc'd
 */
UArray_T get_rgb_blocks(UArray_T rgb_blocks, ppm image)
{
        assert(rgb_blocks != NULL && image != NULL);

        int width   = image->width;
        int height  = image->height;
        const struct A2Methods_T m = *(image->methods);

        int cell = 0;
        for (int row = 0; row < height; row += 2) {
                for (int col = 0; col < width; col += 2) {
                        RGB_block block = (RGB_block)UArray_at(rgb_blocks,
                                                               cell);
                        block->topL = get_rgb_pixel(m.at(image->pixels,
                                      col,     row),     image->denominator);
                        block->topR = get_rgb_pixel(m.at(image->pixels,
                                      col + 1, row),     image->denominator);
                        block->botL = get_rgb_pixel(m.at(image->pixels,
                                      col, row + 1),     image->denominator);
                        block->botR = get_rgb_pixel(m.at(image->pixels,
                                      col + 1, row + 1), image->denominator);

                        cell++;
                }
        }

        return rgb_blocks;
}

/*
 * [Name]:       get_rgb_pixel
 * [Parameters]: 1 Pnm_rgb (RGB value of px), 1 float (denominator to scale
 *               pixel)
 * [Return]:     Scaled RGB pixel
 * [Purpose]:    Copies RGB value from pixel pnm in image into a scaled RGB_px
 *               (that will be stored in an RGB_block)
 *               Note: values in RGB_px are of the range [0, 1]
 * [Errors]:     CRE if pnm is NULL or not malloc'd
 */
RGB_px get_rgb_pixel(Pnm_rgb pnm, float denom)
{
        assert(pnm != NULL);

        RGB_px pixel;
        NEW(pixel);

        pixel->r = (float)pnm->red   / denom;
        pixel->g = (float)pnm->green / denom;
        pixel->b = (float)pnm->blue  / denom;

        return pixel;
}
/* -------------------------------------------- */

/* -------------------------------------------- *
 *                 XYZ / RGB                    |
 * v------------------------------------------v */
/*
 * [Name]:       rgb_xyz
 * [Parameters]: 2 UArrays (<output>: xyz_blocks, <input>: rgb_blocks)
 * [Return]:     xyz_blocks that have pixel values in XYZ color space
 * [Purpose]:    Converts pixel values from RGB color space in rgb_blocks to
 *               XYZ color space in xyz_blocks
 * [Errors]:     CRE if any parameter is NULL or not malloc'd
 */
static UArray_T rgb_xyz(UArray_T xyz_blocks, UArray_T rgb_blocks)
{
        assert(xyz_blocks != NULL && rgb_blocks != NULL);

        for (int i = 0; i < UArray_length(rgb_blocks); i++) {
                XYZ_block xyz = UArray_at(xyz_blocks, i);
                RGB_block rgb = UArray_at(rgb_blocks, i);

                xyz = RGB_to_XYZ(rgb, xyz);
        }

        return xyz_blocks;
}
/* ^------------------------------------------^ */

/* -------------------------------------------- *
 *                  CHROMA                     |
 * v------------------------------------------v */
/*
 * [Name]:       chroma
 * [Parameters]: 2 UArrays (<output>: bit_blocks, <input>: xyz_blocks)
 * [Return]:     bit_blocks that have chroma values converted to their bit
 *               representations, and luma values unchanged
 * [Purpose]:    Converts pixel chroma values from XYZ color space in xyz_blocks
 *               to their bit representations in bit_blocks
 * [Errors]:     CRE if any parameter is NULL or not malloc'd
 */
static UArray_T chroma(UArray_T bit_blocks, UArray_T xyz_blocks)
{
        assert(bit_blocks != NULL && xyz_blocks != NULL);

        for (int i = 0; i < UArray_length(xyz_blocks); i++) {
                XYZ_block xyz = UArray_at(xyz_blocks, i);
                bit_block bit = UArray_at(bit_blocks, i);

                bit = chroma_to_bit(xyz, bit);
        }

        return bit_blocks;
}
/* ^------------------------------------------^ */

/* -------------------------------------------- *
 *                   LUMA                       |
 * v------------------------------------------v */
/*
 * [Name]:       luma
 * [Parameters]: 2 UArrays (<output>: bit_blocks, <input>: xyz_blocks)
 * [Return]:     bit_blocks that have luma values converted to their bit
 *               representations, and chroma values unchanged
 * [Purpose]:    Converts pixel luma values from XYZ color space in xyz_blocks
 *               to their bit representations (DCT space) in bit_blocks
 * [Errors]:     CRE if any parameter is NULL or not malloc'd
 */
static UArray_T luma(UArray_T bit_blocks, UArray_T xyz_blocks)
{
        assert(bit_blocks != NULL && xyz_blocks != NULL);

        for (int i = 0; i < UArray_length(xyz_blocks); i++) {
                XYZ_block xyz = UArray_at(xyz_blocks, i);
                bit_block bit = UArray_at(bit_blocks, i);

                bit = luma_to_bit(xyz, bit);
        }

        return bit_blocks;
}
/* ^------------------------------------------^ */

/* -------------------------------------------- *
 *                  PIXPACK                     |
 * v------------------------------------------v */
/*
 * [Name]:       pixpack
 * [Parameters]: 2 UArrays (<output>: codewords, <input>: bit_blocks)
 * [Return]:     A UArray of codewords, where each codeword has the bit fields
 *               of a 2x2 pixel packed into a 32-bit representation
 * [Purpose]:    Compresses bit representations of pixel values in bit_blocks to
 *               a 32-bit codeword
 * [Errors]:     CRE if any parameter is NULL or not malloc'd
 */
static UArray_T pixpack(UArray_T codewords, UArray_T bit_blocks)
{
        assert(codewords != NULL && bit_blocks != NULL);

        for (int i = 0; i < UArray_length(bit_blocks); i++) {
                uint32_t *buf = UArray_at(codewords, i);
                *buf = pack(UArray_at(bit_blocks, i));
        }

        return codewords;
}
/* ^------------------------------------------^ */

/* -------------------------------------------- *
 *                   WRITE                      |
 * v------------------------------------------v */
/*
 * [Name]:       write
 * [Parameters]: 1 UArray (codewords), 2 unsigned integers (width, height)
 * [Return]:     void
 * [Purpose]:    Prints compressed image stored in codewords to standard output,
 *               with a standard file header (in row-major, big-endian order)
 * [Errors]:     CRE if codewords is NULL
 */
static void write(UArray_T codewords, unsigned width, unsigned height)
{
        assert(codewords != NULL);

        printf("COMP40 Compressed image format 2\n%u %u\n", width, height);

        for (int i = 0; i < UArray_length(codewords); i++) {
                uint32_t *codeword = UArray_at(codewords, i);

                for (int j = 4; j > 0; j--) {
                        char c = extract_char(*codeword, j - 1);
                        putchar(c);
                }
        }
}
/* ^------------------------------------------^ */

/* -------------------------------------------- *
 *                  FREE                        |
 * v------------------------------------------v */
/*
 * [Name]:       free_c
 * [Parameters]: 4 UArrays, 1 ppm
 * [Return]:     void
 * [Purpose]:    Frees the given UArrays and ppm
 * [Errors]:     None
 */
static void free_c(UArray_T rgb_blocks, UArray_T xyz_blocks, UArray_T bit_blocks,
                   UArray_T codewords,  ppm image)
{
        free_rgb_c      (rgb_blocks);
        free_xyz_c      (xyz_blocks);
        free_bit_c      (bit_blocks);
        free_codewords_c(codewords);
        Pnm_ppmfree     (&image);
}

/*
 * [Name]:       free_rgb_c
 * [Parameters]: 1 UArray (rgb_blocks)
 * [Return]:     void
 * [Purpose]:    Frees the given UArray of rgb_blocks and the pixels within
 *               each RGB_block
 * [Errors]:     None
 */
void free_rgb_c(UArray_T rgb_blocks)
{
        for (int i = 0; i < UArray_length(rgb_blocks); i++) {
                RGB_block block = UArray_at(rgb_blocks, i);
                free_RGB_block(block);
        }

        UArray_free(&rgb_blocks);
}

/*
 * [Name]:       free_xyz_c
 * [Parameters]: 1 UArray (xyz_blocks)
 * [Return]:     void
 * [Purpose]:    Frees the given UArray of xyz_blocks and the pixels within
 *               each XYZ_block
 * [Errors]:     None
 */
void free_xyz_c(UArray_T xyz_blocks)
{
        for (int i = 0; i < UArray_length(xyz_blocks); i++) {
                XYZ_block block = UArray_at(xyz_blocks, i);
                free_XYZ_block(block);
        }

        UArray_free(&xyz_blocks);
}

/*
 * [Name]:       free_bit_c
 * [Parameters]: 1 UArray (bit_blocks)
 * [Return]:     void
 * [Purpose]:    Frees the given UArray of bit_blocks
 * [Errors]:     None
 */
void free_bit_c(UArray_T bit_blocks)
{
        UArray_free(&bit_blocks);
}

/*
 * [Name]:       free_codewords_c
 * [Parameters]: 1 UArray (codewords)
 * [Return]:     void
 * [Purpose]:    Frees the given UArray of codewords
 * [Errors]:     None
 */
void free_codewords_c(UArray_T codewords)
{
        UArray_free(&codewords);
}
/* ^------------------------------------------^ */

/* Private struct with function pointers */
static struct ImageMethods_T compress_struct = {
        new_blocks,
        read,
        rgb_xyz,
        chroma,
        luma,
        pixpack,
        write,
        free_c,
};

/* The Payoff: Exported pointer to the struct */
ImageMethods_T compress = &compress_struct;
/* -------------------------------------------- */