/*
 *      pixelblock.h
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *      HW4 Arith
 *
 *      - File that defines the pixel & block structs in use throughout 
 *        our program
 *      - File also defines various constants
 */

#ifndef PIXELBLOCK_INCLUDED
#define PIXELBLOCK_INCLUDED

#include "pnm.h"

/* -- GLOBAL CONSTANTS -- */
/* Range of RGB values in any output will be [0, 255] */
static const float RGB_MAX      = 255.0;

/* Size of bit fields, in bits */
static const int   A_WIDTH      = 6;
static const int   B_WIDTH      = 6;
static const int   C_WIDTH      = 6;
static const int   D_WIDTH      = 6;
static const int   PB_WIDTH     = 4;
static const int   PR_WIDTH     = 4;

static const int   BITS_IN_BYTE = 8;
/* ---------------------- */

/* Struct Definitions for Individual & 2x2 Pixel Values */
/* RGB Pixel */
typedef struct RGB_px {
        float r, g, b;
} *RGB_px;

/* 2x2 RGB Pixels */
typedef struct RGB_block {
        RGB_px topL, topR, botL, botR;
} *RGB_block;

/* XYZ Pixel */
typedef struct XYZ_px {
        float luma, Pb, Pr;
} *XYZ_px;

/* 2x2 XYZ Pixels */
typedef struct XYZ_block {
        XYZ_px topL, topR, botL, botR;
} *XYZ_block;

/*  2x2 Pixels in Bit Block */
typedef struct bit_block {
        unsigned a, Pb, Pr;
          signed b, c, d;
} *bit_block;

#endif /* PIXELBLOCK_INCLUDED */