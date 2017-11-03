/*
 *      imagemethods.h
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *      HW4 Arith
 *
 *      - Header file declaring client-accessible polymorphic method suite for
 *        manipulating images
 *      - Method suite allows for compression or decompression of image,
 *        converting between a portable pixmap and a compressed COMP40 image
 */

#ifndef IMAGEMETHODS_INCLUDED
#define IMAGEMETHODS_INCLUDED

#include "pnm.h"
#include "uarray.h"

/* Exported method suite with pointers to the image manipulation methods */
typedef struct ImageMethods_T {
        
        UArray_T(*new_blocks)(unsigned length, unsigned size);
        UArray_T(*read)      (UArray_T output, Pnm_ppm image);
        UArray_T(*rgb_xyz)   (UArray_T output, UArray_T input);
        UArray_T(*chroma)    (UArray_T output, UArray_T input);
        UArray_T(*luma)      (UArray_T output, UArray_T input);
        UArray_T(*pixpack)   (UArray_T output, UArray_T input);
        void    (*write)     (UArray_T input, unsigned width, unsigned height);
        void    (*free)      (UArray_T rgb_blocks, UArray_T xyz_blocks,
                              UArray_T bit_blocks, UArray_T codewords,
                              Pnm_ppm image);

} *ImageMethods_T;

/* Exported method types: compression and decompression */
extern ImageMethods_T compress;
extern ImageMethods_T decompress;

#endif /* IMAGEMETHODS_INCLUDED */