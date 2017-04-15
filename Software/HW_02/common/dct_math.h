//
//  dct_math.h
//  mjpeg423app
//
//  Created by Rodolfo Pellizzoni on 12/28/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef mjpeg423app_dct_math_h
#define mjpeg423app_dct_math_h

#include "mjpeg423_types.h"

/*
 * Macros for handling fixed-point arithmetic.
 *
 * All values are expected to be of type int32_t.
 */

#define ONE	((int32_t) 1)

/* We assume that right shift corresponds to signed division by 2 with
 * rounding towards minus infinity.  This is correct for typical "arithmetic
 * shift" instructions that shift in copies of the sign bit.  But some
 * C compilers implement >> with an unsigned shift.  For these machines you
 * must define RIGHT_SHIFT_IS_UNSIGNED.
 * RIGHT_SHIFT provides a proper signed right shift of an INT32 quantity.
 * It is only applied with constant shift counts.  SHIFT_TEMPS must be
 * included in the variables of any routine using RIGHT_SHIFT.
 */

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define SHIFT_TEMPS	int32_t shift_temp;
#define RIGHT_SHIFT(x,shft)  \
((shift_temp = (x)) < 0 ? \
(shift_temp >> (shft)) | ((~((int32_t) 0)) << (32-(shft))) : \
(shift_temp >> (shft)))
#else
#define SHIFT_TEMPS
#define RIGHT_SHIFT(x,shft)	((x) >> (shft))
#endif

/* Descale and correctly round an INT32 value that's scaled by N bits.
 * We assume RIGHT_SHIFT rounds towards minus infinity, so adding
 * the fudge factor is correct for either sign of X.
 */

#define DESCALE(x,n)  RIGHT_SHIFT((x) + (ONE << ((n)-1)), n)

#define CONST_BITS  13
#define PASS1_BITS  2

#define FIX_0_298631336  ((int32_t)  2446)	/* FIX(0.298631336) */
#define FIX_0_390180644  ((int32_t)  3196)	/* FIX(0.390180644) */
#define FIX_0_541196100  ((int32_t)  4433)	/* FIX(0.541196100) */
#define FIX_0_765366865  ((int32_t)  6270)	/* FIX(0.765366865) */
#define FIX_0_899976223  ((int32_t)  7373)	/* FIX(0.899976223) */
#define FIX_1_175875602  ((int32_t)  9633)	/* FIX(1.175875602) */
#define FIX_1_501321110  ((int32_t)  12299)	/* FIX(1.501321110) */
#define FIX_1_847759065  ((int32_t)  15137)	/* FIX(1.847759065) */
#define FIX_1_961570560  ((int32_t)  16069)	/* FIX(1.961570560) */
#define FIX_2_053119869  ((int32_t)  16819)	/* FIX(2.053119869) */
#define FIX_2_562915447  ((int32_t)  20995)	/* FIX(2.562915447) */
#define FIX_3_072711026  ((int32_t)  25172)	/* FIX(3.072711026) */


/* Multiply an INT32 variable by an INT32 constant to yield an INT32 result.
 * For 8-bit samples with the recommended scaling, all the variable
 * and constant values involved are no more than 16 bits wide, so a
 * 16x16->32 bit multiply can be used instead of a full 32x32 multiply.
 */

#define MULTIPLY16C16(var,const)  (((int16_t) (var)) * ((int16_t) (const)))

//#define MULTIPLY(var,const)  MULTIPLY16C16(var,const)
#define MULTIPLY(var,const)  ((var) * (const))

#define DCTSIZE 8


#endif
