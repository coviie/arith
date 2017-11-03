/*
 *      rgb_xyz.c
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *      HW4 Arith
 *
 *      - Component file defining all extern and helper functions for the
 *        rgb_xyz component
 *      - Component converts pixels in a 2x2 block between RGB and XYZ color
 *        spaces (note: XYZ encodes luminance and chromatic values)
 *      - Component-wide invariants:
 *              ~ 0 <= luma (Y) <= 1
 *              ~ -0.5 <= Pb <= 0.5
 *              ~ -0.5 <= Pr <= 0.5
 *              ~ Blocks passed in as "input" are not modified
 */

#include <stdio.h>
#include <stdlib.h>

#include "assert.h"
#include "mem.h"
#include "rgb_xyz.h"

/* -- COMPRESS HELPER FUNCTIONS -- */
XYZ_px to_float    (RGB_px rgb);
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* -- DECOMPRESS HELPER FUNCTIONS -- */
RGB_px to_int      (XYZ_px xyz);
RGB_px quantize_RGB(float r, float g, float b);
float  scale_RGB   (float value);
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/*---------------------------------------------------------------
 |                 COMPRESS CONVERSION FUNCTIONS                |
 *--------------------------------------------------------------*/
/*
 * [Name]:       RGB_to_XYZ
 * [Parameters]: 1 RGB_block, 1 XYZ_block
 *               Note: Range of scaled rgb values should be [0, 1]
 * [Return]:     XYZ_block, overwritten with values converted from RGB_block
 *               Note: Overwrites existing values in xyz
 * [Purpose]:    Converts pixel values in 2x2 block from RGB to XYZ color space
 *               Note: Does not modify values in rgb
 *                     Memory for pixels need to be freed (free_XYZ_block)
 * [Errors]:     CRE if any block is NULL or has not been malloc'd
 *               URE if client loses pointer to blocks, or if rgb values are
 *                   not scaled properly
 */
XYZ_block RGB_to_XYZ(RGB_block rgb, XYZ_block xyz)
{
        assert(rgb != NULL && xyz != NULL);

        xyz->topL = to_float(rgb->topL);
        xyz->topR = to_float(rgb->topR);
        xyz->botL = to_float(rgb->botL);
        xyz->botR = to_float(rgb->botR);

        return xyz;
}

/*
 * [Name]:       to_float
 * [Parameters]: 1 RGB_px
 *               Note: Range of scaled rgb values is [0, 1]
 * [Return]:     XYZ_px, containing values converted from RGB_px
 * [Purpose]:    Converts pixel value from RGB to XYZ color space
 *               Note: Does not modify values in rgb
 *                     Memory for pixel needs to be freed (free_XYZ_block)
 * [Errors]:     CRE if pixel is NULL or has not been malloc'd
 *               URE if rgb values are not scaled properly
 */
XYZ_px to_float(RGB_px rgb)
{
        assert(rgb != NULL);

        XYZ_px xyz;
        NEW(xyz);

        float r = rgb->r;
        float g = rgb->g;
        float b = rgb->b;

        xyz->luma = 0.299     * r + 0.587    * g + 0.114    * b;
        xyz->Pb   = -0.168736 * r - 0.331264 * g + 0.5      * b;
        xyz->Pr   = 0.5       * r - 0.418688 * g - 0.081312 * b;

        return xyz;
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/*---------------------------------------------------------------
 |                DECOMPRESS CONVERSION FUNCTIONS               |
 *--------------------------------------------------------------*/
/*
 * [Name]:       XYZ_to_RGB
 * [Parameters]: 1 XYZ_block, 1 RGB_block
 *               Note: Overwrites existing values in rgb
 * [Return]:     RGB_block, overwritten with values converted from XYZ_block
 *               Note: Range of rgb values is [0, RGB_MAX]
 * [Purpose]:    Converts pixel values in 2x2 block from XYZ to RGB color space
 *               Note: Does not modify values in xyz
 *                     Memory for pixels need to be freed (free_RGB_block)
 * [Errors]:     CRE if any block is NULL or has not been malloc'd
 *               URE if client loses pointers to any block
 */
RGB_block XYZ_to_RGB(XYZ_block xyz, RGB_block rgb)
{
        assert(rgb != NULL && xyz != NULL);

        rgb->topL = to_int(xyz->topL);
        rgb->topR = to_int(xyz->topR);
        rgb->botL = to_int(xyz->botL);
        rgb->botR = to_int(xyz->botR);

        return rgb;
}

/*
 * [Name]:       to_float
 * [Parameters]: 1 XYZ_px
 * [Return]:     RGB_px, containing values converted from XYZ_px
 *               Note: Range of rgb values is [0, RGB_MAX]
 * [Purpose]:    Converts pixel value from XYZ to RGB color space
 *               Note: Does not modify values in xyz
 *                     Memory for pixel needs to be freed (free_RGB_block)
 * [Errors]:     CRE if pixel is NULL or has not been malloc'd
 *               URE if client loses pointer to returned pixel
 */
RGB_px to_int(XYZ_px xyz)
{
        assert(xyz != NULL);

        float y  = xyz->luma;
        float pb = xyz->Pb;
        float pr = xyz->Pr;
        float r, g, b;

        r = 1.0 * y + 0.0      * pb + 1.402    * pr;
        g = 1.0 * y - 0.344136 * pb - 0.714136 * pr;
        b = 1.0 * y + 1.772    * pb + 0.0      * pr;

        return quantize_RGB(r, g, b);
}

/*
 * [Name]:       quantize_RGB
 * [Parameters]: 3 floats (red, green, blue values)
 * [Return]:     RGB_px, containing quantized RGB values
 *               Note: Range of rgb values is [0, RGB_MAX]
 * [Purpose]:    Converts pixel values from scaled to quantized RGB
 *               Note: Memory for pixel needs to be freed (free_RGB_block)
 * [Errors]:     URE if client loses pointer to returned pixel
 */
RGB_px quantize_RGB(float r, float g, float b)
{
        RGB_px rgb;
        NEW(rgb);

        rgb->r = scale_RGB(r);
        rgb->g = scale_RGB(g);
        rgb->b = scale_RGB(b);

        return rgb;
}

/*
 * [Name]:       scale_RGB
 * [Parameters]: 1 float
 * [Return]:     float, containing quantized RGB value
 *               Note: Range of return value is [0, RGB_MAX]
 * [Purpose]:    Converts RGB value from scaled to quantized RGB
 *               Note: Any out-of-range value will be coded to 0 or RGB_MAX
 * [Errors]:     None
 */
float scale_RGB(float value)
{
        value *= RGB_MAX;

        if (value > RGB_MAX) {
                value = RGB_MAX;
        } else if (value < 0) {
                value = 0;
        }

        return value;
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/*---------------------------------------------------------------
 |                 MEMORY DEALLOCATION FUNCTIONS                |
 *--------------------------------------------------------------*/
/*
 * [Name]:       free_XYZ_block
 * [Parameters]: 1 XYZ_block
 * [Return]:     Void
 * [Purpose]:    Frees the memory for each pixel in the xyz block
 * [Errors]:     CRE if block is NULL or has already been dealloc'd
 *               URE if any pixel has already been dealloc'd
 */
void free_XYZ_block(XYZ_block xyz)
{
        assert(xyz != NULL);

        FREE(xyz->topL);
        FREE(xyz->topR);
        FREE(xyz->botL);
        FREE(xyz->botR);
}

/*
 * [Name]:       free_RGB_block
 * [Parameters]: 1 RGB_block
 * [Return]:     Void
 * [Purpose]:    Frees the memory for each pixel in the rgb block
 * [Errors]:     CRE if block is NULL or has already been dealloc'd
 *               URE if any pixel has already been dealloc'd
 */
void free_RGB_block(RGB_block rgb)
{
        assert(rgb != NULL);

        FREE(rgb->topL);
        FREE(rgb->topR);
        FREE(rgb->botL);
        FREE(rgb->botR);
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */