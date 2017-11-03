/*
 *      rgb_xyz.h
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *
 *      - Header file declaring client-accessible functions for the
 *        rgb_xyz component
 *      - Component converts pixels in a 2x2 block between RGB and XYZ color
 *        spaces (note: XYZ encodes luminance and chromatic values)
 */

#ifndef RGBXYZ_INCLUDED
#define RGBXYZ_INCLUDED

#include "pixelblock.h"

/* -- CONVERSION FUNCTIONS -- */
/*
 * Overwrites old values in xyz with conversions from rgb, then returns xyz
 * CRE: parameters cannot be NULL
 */
extern XYZ_block RGB_to_XYZ(RGB_block rgb, XYZ_block xyz);

/*
 * Overwrites old values in rgb with conversions from xyz, then returns rgb
 * CRE: parameters cannot be NULL
 */
extern RGB_block XYZ_to_RGB(XYZ_block xyz, RGB_block rgb);
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* -- MEMORY FUNCTIONS -- */
/*
 * Frees the memory allocated for pixels within a pixel block
 * CRE: parameters cannot be NULL
 */
extern void free_XYZ_block(XYZ_block xyz);
extern void free_RGB_block(RGB_block rgb);
/* ^^^^^^^^^^^^^^^^^^^^^^ */

#endif /* RGBXYZ_INCLUDED */