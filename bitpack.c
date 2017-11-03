/*
 *      bitpack.c
 *      by Jia Wen Goh (jgoh01) & Sean Ong (song02), 10/20/2017
 *      HW4 Arith
 *
 *      - This interface manipulates bit fields within a 64-bit/8-byte word.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arith40.h"
#include "assert.h"
#include "bitpack.h"
#include "except.h"

/* -- Bit Constants -- */
const unsigned int MAX_BIT = 64;
const uint64_t     MAX_64U = 0xffffffffffffffff;
const  int64_t     MAX_64  = 0x7fffffffffffffff;
const  int64_t     MIN_64  = 0x8000000000000000; // ~MAX_64
/* ^^^^^^^^^^^^^^^^^^^ */

/* -- Direction Constants -- */
const int LEFT  = 0;
const int RIGHT = 1;
/* ^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* -- EXCEPTIONS & ASSERTIONS -- */
Except_T Bitpack_Overflow = { "Overflow packing bits" };
void width_lsb_check(unsigned width, unsigned lsb);
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* -- BITWISE SHIFT HELPER FUNCTIONS -- */
uint64_t shiftu(uint64_t n, unsigned magnitude, int direction);
 int64_t shifts( int64_t n, unsigned magnitude, int direction);
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/*---------------------------------------------------------------
 |                    WIDTH TEST FUNCTIONS                      |
 *--------------------------------------------------------------*/
/* 
 * [Name]:       Bitpack_fitsu
 * [Parameters]: 1 uint64_t, 1 unsigned
 * [Returns]:    boolean; if the given uint64_t n can fit in width bits, then 
 *               return true (1), else return false (0)
 * [Purpose]:    Tests if the given unsigned integer can be represented in 
 *               width-bits
 * [Errors]:     CRE if the specified width is more than the max possible 
 *               bits (64)
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        assert(width <= MAX_BIT);

        uint64_t max;

        if (width == MAX_BIT) {
                max = MAX_64U;
        } else {
                max = shiftu(1, width, LEFT) - 1;
        }

        if (n <= max) {
                return true;
        } else {
                return false;
        }
}

/* 
 * [Name]:       Bitpack_fitss
 * [Parameters]: 1 int64_t, 1 unsigned
 * [Returns]:    boolean; if the given int64_t n can fit in width bits, then 
 *               return true (1), else return false (0)
 * [Purpose]:    Tests if the given signed integer can be represented in 
 *               width-bits
 * [Errors]:     CRE if the specified width is more than the max possible 
 *               bits (64)
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
        assert(width <= MAX_BIT);

        if (width == 0 || n > MAX_64) {
                return false;
        } else if (n >= 0 && n <= MAX_64) {
                return Bitpack_fitsu(n, width - 1);
        } else {
                int64_t min = -1 * (shifts(1, width - 1, LEFT));
                if (n >= min) {
                        return true;
                } else {
                        return false;
                }
        }
}

/*---------------------------------------------------------------
 |                 FIELD-EXTRACTION FUNCTIONS                   |
 *--------------------------------------------------------------*/
/* 
 * [Name]:       Bitpack_getu
 * [Parameters]: 1 uint64_t, 2 unsigned (width & lsb)
 * [Returns]:    The uint64_t of given-width located from the given least 
 *               significant bits
 * [Purpose]:    Extracts an unsigned int from a word with the given width
 *               from the least significant bit supplied
 *               Note: Does not modify original word
 * [Errors]:     CRE thrown by width_lsb_check if the specified width & lsb are
 *               invalid (as stated in width_lsb_check)
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        width_lsb_check(width, lsb);

        uint64_t mask = shiftu(shiftu(1, width, LEFT) - 1, lsb, LEFT);

        return shiftu(mask & word, lsb, RIGHT);
}

/* 
 * [Name]:       Bitpack_gets
 * [Parameters]: 1 uint64_t, 2 unsigned (width & lsb)
 * [Returns]:    The int64_t of given-width located from the given least 
 *               significant bits
 * [Purpose]:    Extracts a signed int from a word with the given width
 *               from the least significant bit supplied
 *               Note: Does not modify original word
 * [Errors]:     CRE thrown by width_lsb_check if the specified width & lsb are
 *               invalid (as stated in width_lsb_check)
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        width_lsb_check(width, lsb);

        int64_t result      = Bitpack_getu(word, width, lsb);
        int64_t is_negative = shifts(1, width - 1, LEFT);

        if ((result & is_negative) != false) {
                int64_t offset = shifts(MIN_64, MAX_BIT - width, RIGHT);
                        result = result | offset;
        }

        return result;
}

/*---------------------------------------------------------------
 |                   FIELD-UPDATE FUNCTIONS                     |
 *--------------------------------------------------------------*/
/* 
 * [Name]:       Bitpack_newu
 * [Parameters]: 2 uint64_t (original word & value to replace), 2 unsigned 
 *               (width & lsb)
 * [Returns]:    A uint64_t (bit representation) that is identical to the 
 *               original word, but with the field of width-size from lsb 
 *               replaced by the width-bit representation of value
 * [Purpose]:    "Replace" a portion of the original word with the given value
 *               Note: word is modified on return, and is only modified from
 *                     lsb to lsb + width
 * [Errors]:     CRE thrown by width_lsb_check if the specified width & lsb are
 *               invalid (as stated in width_lsb_check)
 *               CRE if the supplied value doesn't fit in width-unsigned bits
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                      uint64_t value)
{
        width_lsb_check(width, lsb);

        uint64_t max = shiftu(1, width, LEFT) - 1;
        if (value > max) {
                RAISE(Bitpack_Overflow);
        }

        uint64_t mask  = shiftu(MAX_64U, width + lsb, LEFT) |
                         (shiftu(1, lsb, LEFT) - 1);
                 word  = word & mask;
                 value = shiftu(value, lsb, LEFT);

        return (value | word);
}

/* 
 * [Name]:       Bitpack_news
 * [Parameters]: 1 uint64_t (original word), 2 unsigned (width & lsb), 1 int64_t
 *               (value to replace)
 * [Returns]:    A uint64_t (bit representation) that is identical to the 
 *               original word, but with the field of width-size from lsb 
 *               replaced by the width-bit representation of value
 * [Purpose]:    "Replace" a portion of the original word with the given value
 *               Note: word is modified on return, and is only modified from
 *                     lsb to lsb + width
 * [Errors]:     CRE thrown by width_lsb_check if the specified width & lsb are
 *               invalid (as stated in width_lsb_check)
 *               CRE if the supplied value doesn't fit in width-signed bits
 */
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,
                      int64_t value)
{
        width_lsb_check(width, lsb);

        int64_t max = shiftu(1, width - 1, LEFT) - 1;
        int64_t min = -1 * (shifts(1, width - 1, LEFT));

        if (value > max || value < min) {
                RAISE(Bitpack_Overflow);
        }

        int64_t offset = shifts(1, width, LEFT) - 1;
                 value = value & offset;

        return Bitpack_newu(word, width, lsb, value);
}

/*---------------------------------------------------------------
 |                 ASSERTION HELPER FUNCTIONS                   |
 *--------------------------------------------------------------*/
/* 
 * [Name]:       width_lsb_check
 * [Parameters]: 2 unsigned
 * [Returns]:    void (but raises CRE given conditions listed below)
 * [Purpose]:    Asserts/raises a CRE if the given dimensions result are invalid
 *               (i.e. results in an index that will go out of range of MAX_BIT)
 * [Errors]:     CRE if:
 *                   - width >  MAX_BIT (64)
 *                   - lsb   >= MAX_BIT, and
 *                   - (width + lsb) > MAX_BIT
 */
void width_lsb_check(unsigned width, unsigned lsb)
{
        assert(width <= MAX_BIT &&
               lsb   <  MAX_BIT &&
               (width + lsb) <= MAX_BIT);
}

/*---------------------------------------------------------------
 |                   SHIFT HELPER FUNCTIONS                     |
 *--------------------------------------------------------------*/
/* 
 * [Name]:       shiftu
 * [Parameters]: 1 uint64_t (value to shift), 1 unsigned, 1 int (direction - 
 *               a constant will be passed in)
 * [Returns]:    The shifted value based on the given magnitude & direction
 *               Note: this function addresses shifting by 64-bits because a  
 *                     shift by 64 on Intel hardware does not do anything
 * [Purpose]:    Shifts the given unsigned 64-bit int
 * [Errors]:     None 
 */
uint64_t shiftu(uint64_t n, unsigned magnitude, int direction)
{
        if (magnitude == MAX_BIT) {
                return 0;
        } else {
                if (direction == LEFT) {
                        return (n << magnitude);
                } else {
                        return (n >> magnitude);
                }
        }
}

/* 
 * [Name]:       shifts
 * [Parameters]: 1 int64_t (value to shift), 1 unsigned, 1 int (direction - 
 *               a constant will be passed in)
 * [Returns]:    The shifted value based on the given magnitude & direction
 *               Note: this function addresses shifting by 64-bits because a  
 *                     shift by 64 on Intel hardware does not do anything
 * [Purpose]:    Shifts the given signed 64-bit int
 * [Errors]:     None 
 */
int64_t shifts(int64_t n, unsigned magnitude, int direction)
{
        if (magnitude == MAX_BIT) {
                if (direction == LEFT) {
                        return 0;
                } else {
                        return -1;
                }
        } else {
                if (direction == LEFT) {
                        return (n << magnitude);
                } else {
                        return (n >> magnitude);
                }
        }
}