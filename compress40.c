/*
 *      compress40.c
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *
 *      - File that defines compress/decompress functions for 40image
 *      - Compress: Reads an uncompressed portable pixmap from the input stream
 *                  and compresses it into a COMP40 format on standard output
 *      - Decompress: Reads a compressed COMP40 format image from the input
 *                    stream and decompresses it into an uncompressed portable
 *                    pixmap on standard output
 *      - Invariants:
 *              ~ Image (compressed or decompressed) is never modified
 *              ~ Num of 2x2 blocks in img = (img_width / 2) * (img_height / 2)
 */

#include <stdio.h>
#include <stdlib.h>

#include "a2methods.h"
#include "a2plain.h"
#include "assert.h"
#include "compress40.h"
#include "imagemethods.h"
#include "pixpack.h"
#include "uarray.h"

/* -- struct Pnm_ppm is from pnm.h -- */
typedef struct Pnm_ppm *ppm;

/*--------------------------------------------------------------*
 |                      COMPRESS FUNCTION                       |
 *--------------------------------------------------------------*/
/*
 * [Name]:       compress40
 * [Parameters]: 1 FILE* (input)
 * [Return]:     void
 * [Purpose]:    Compresses image on input stream and sends it on standard
 *               output in the COMP40 compressed image format
 *               Note: Does not modify or close input
 *                     Sets ImageMethods to compress, then calls the respective
 *                      image functions
 * [Errors]:     CRE if input is NULL
 */
void compress40(FILE* input)
{
        assert(input != NULL);

        /* Initializing image and methods */
        A2Methods_T    A2_m  = uarray2_methods_plain;
        ImageMethods_T img_m = compress;
        ppm            image = Pnm_ppmread(input, A2_m);

        /* Allocating memory for UArrays */
        unsigned len        = (image->width / 2) * (image->height / 2);
        UArray_T rgb_blocks = img_m->new_blocks(len, sizeof(struct RGB_block));
        UArray_T xyz_blocks = img_m->new_blocks(len, sizeof(struct XYZ_block));
        UArray_T bit_blocks = img_m->new_blocks(len, sizeof(struct bit_block));
        UArray_T codewords  = img_m->new_blocks(len, sizeof(        uint32_t));

        /* Image methods */
        rgb_blocks = img_m->read   (rgb_blocks, image);
        xyz_blocks = img_m->rgb_xyz(xyz_blocks, rgb_blocks);
        bit_blocks = img_m->chroma (bit_blocks, xyz_blocks);
        bit_blocks = img_m->luma   (bit_blocks, xyz_blocks);
        codewords  = img_m->pixpack(codewords,  bit_blocks);
        /* AFTER THIS POINT: Image has been compressed */

        img_m->write(codewords, image->width, image->height);
        img_m->free (rgb_blocks, xyz_blocks, bit_blocks, codewords, image);
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/*--------------------------------------------------------------*
 |                     DECOMPRESS FUNCTION                      |
 *--------------------------------------------------------------*/
/*
 * [Name]:       decompress40
 * [Parameters]: 1 FILE* (input)
 * [Return]:     void
 * [Purpose]:    Decompresses image on input stream and sends it on standard
 *               output in a portable pixmap format
 *               Note: Does not modify or close input
 *                     Sets ImageMethods to decompress, then calls respective
 *                      image functions
 * [Errors]:     CRE if input is NULL
 */
void decompress40(FILE* input)
{
        assert(input != NULL);        
        
        /* Setting methods */
        ImageMethods_T img_m = decompress;
        
        /* Reading file header */
        unsigned height, width;
        int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u",
                          &width, &height);
        assert(read == 2);
        int c = getc(input);
        assert(c == '\n');

        /* Allocating memory for UArrays*/
        unsigned len = (width / 2) * (height / 2);
        UArray_T rgb_blocks = img_m->new_blocks(len, sizeof(struct RGB_block));
        UArray_T xyz_blocks = img_m->new_blocks(len, sizeof(struct XYZ_block));
        UArray_T bit_blocks = img_m->new_blocks(len, sizeof(struct bit_block));
        UArray_T codewords  = img_m->new_blocks(len, sizeof(        uint32_t));

        /* Initializing compressed image */
        for (int i = 0; i < UArray_length(codewords); i++) {
                uint32_t *codeword = UArray_at(codewords, i);

                for (int j = sizeof(uint32_t); j > 0; j--) {
                        char c = getc(input);
                        *codeword = store_char(c, *codeword, j - 1);
                }
        }

        /* Image methods */
        bit_blocks = img_m->pixpack(bit_blocks, codewords);
        xyz_blocks = img_m->luma   (xyz_blocks, bit_blocks);
        xyz_blocks = img_m->chroma (xyz_blocks, bit_blocks);
        rgb_blocks = img_m->rgb_xyz(rgb_blocks, xyz_blocks);
        /* AFTER THIS POINT: Image has been decompressed */
        
        img_m->write(rgb_blocks, width, height);
        img_m->free (rgb_blocks, xyz_blocks, bit_blocks, codewords, NULL);
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */