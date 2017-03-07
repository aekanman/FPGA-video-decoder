//
//  idct.c
//  mjpeg423app
//
//  Created by Rodolfo Pellizzoni on 12/28/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include "mjpeg423_decoder.h"
#include "../common/dct_math.h"
#include "../common/util.h"


#ifndef NULL_DCT

/* normalize the result between 0 and 255 */
/* this is required to handle precision errors that might cause the decoded result to fall out of range */
#define NORMALIZE(x) (temp = (x), ( (temp < 0) ? 0 : ( (temp > 255) ? 255 : temp  ) ) )

void idct(pdct_block_t DCAC, pcolor_block_t block)
{
    int32_t tmp0, tmp1, tmp2, tmp3;
    int32_t tmp10, tmp11, tmp12, tmp13;
    int32_t z1, z2, z3, z4, z5;
    int32_t temp;
    DCTELEM* inptr;
    int32_t * wsptr;
    uint8_t* outptr;
    int ctr;
    int32_t workspace[DCTSIZE*DCTSIZE];	/* buffers data between passes */
    SHIFT_TEMPS
    
    /* Pass 1: process columns from input, store into work array. */
    /* Note results are scaled up by sqrt(8) compared to a true IDCT; */
    /* furthermore, we scale the results by 2**PASS1_BITS. */
    
    inptr = DCAC[0];
    wsptr = workspace;
    for (ctr = DCTSIZE; ctr > 0; ctr--) {
        
        /* Even part: reverse the even part of the forward DCT. */
        /* The rotator is sqrt(2)*c(-6). */
        
        z2 = inptr[DCTSIZE*2];
        z3 = inptr[DCTSIZE*6];
        
        z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
        tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
        tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);
        
        z2 = inptr[DCTSIZE*0];
        z3 = inptr[DCTSIZE*4];
        
        tmp0 = (z2 + z3) << CONST_BITS;
        tmp1 = (z2 - z3) << CONST_BITS;
        
        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;
        
        /* Odd part per figure 8; the matrix is unitary and hence its
         * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
         */
        
        tmp0 = inptr[DCTSIZE*7];
        tmp1 =inptr[DCTSIZE*5];
        tmp2 = inptr[DCTSIZE*3];
        tmp3 = inptr[DCTSIZE*1];
        
        z1 = tmp0 + tmp3;
        z2 = tmp1 + tmp2;
        z3 = tmp0 + tmp2;
        z4 = tmp1 + tmp3;
        z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */
        
        tmp0 = MULTIPLY(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
        tmp1 = MULTIPLY(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
        tmp2 = MULTIPLY(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
        tmp3 = MULTIPLY(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
        z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
        z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
        z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
        z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */
        
        z3 += z5;
        z4 += z5;
        
        tmp0 += z1 + z3;
        tmp1 += z2 + z4;
        tmp2 += z2 + z3;
        tmp3 += z1 + z4;
        
        /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */
        
        wsptr[DCTSIZE*0] = (int32_t) DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
        wsptr[DCTSIZE*7] = (int32_t) DESCALE(tmp10 - tmp3, CONST_BITS-PASS1_BITS);
        wsptr[DCTSIZE*1] = (int32_t) DESCALE(tmp11 + tmp2, CONST_BITS-PASS1_BITS);
        wsptr[DCTSIZE*6] = (int32_t) DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
        wsptr[DCTSIZE*2] = (int32_t) DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
        wsptr[DCTSIZE*5] = (int32_t) DESCALE(tmp12 - tmp1, CONST_BITS-PASS1_BITS);
        wsptr[DCTSIZE*3] = (int32_t) DESCALE(tmp13 + tmp0, CONST_BITS-PASS1_BITS);
        wsptr[DCTSIZE*4] = (int32_t) DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);
        
        inptr++;			/* advance pointers to next column */
        wsptr++;
    }
    
    /* Pass 2: process rows from work array, store into output array. */
    /* Note that we must descale the results by a factor of 8 == 2**3, */
    /* and also undo the PASS1_BITS scaling. */
    
    wsptr = workspace;
    for (ctr = 0; ctr < DCTSIZE; ctr++) {
        outptr = block[ctr];
        
        /* Even part: reverse the even part of the forward DCT. */
        /* The rotator is sqrt(2)*c(-6). */
        
        z2 = (int32_t) wsptr[2];
        z3 = (int32_t) wsptr[6];
        
        z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
        tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
        tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);
        
        tmp0 = ((int32_t) wsptr[0] + (int32_t) wsptr[4]) << CONST_BITS;
        tmp1 = ((int32_t) wsptr[0] - (int32_t) wsptr[4]) << CONST_BITS;
        
        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;
        
        /* Odd part per figure 8; the matrix is unitary and hence its
         * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
         */
        
        tmp0 = (int32_t) wsptr[7];
        tmp1 = (int32_t) wsptr[5];
        tmp2 = (int32_t) wsptr[3];
        tmp3 = (int32_t) wsptr[1];
        
        z1 = tmp0 + tmp3;
        z2 = tmp1 + tmp2;
        z3 = tmp0 + tmp2;
        z4 = tmp1 + tmp3;
        z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */
        
        tmp0 = MULTIPLY(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
        tmp1 = MULTIPLY(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
        tmp2 = MULTIPLY(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
        tmp3 = MULTIPLY(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
        z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
        z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
        z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
        z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */
        
        z3 += z5;
        z4 += z5;
        
        tmp0 += z1 + z3;
        tmp1 += z2 + z4;
        tmp2 += z2 + z3;
        tmp3 += z1 + z4;
        
        /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */
        outptr[0] = NORMALIZE(DESCALE(tmp10 + tmp3, CONST_BITS+PASS1_BITS+3));
        outptr[7] = NORMALIZE(DESCALE(tmp10 - tmp3, CONST_BITS+PASS1_BITS+3));
        outptr[1] = NORMALIZE(DESCALE(tmp11 + tmp2, CONST_BITS+PASS1_BITS+3));
        outptr[6] = NORMALIZE(DESCALE(tmp11 - tmp2,CONST_BITS+PASS1_BITS+3));
        outptr[2] = NORMALIZE(DESCALE(tmp12 + tmp1,CONST_BITS+PASS1_BITS+3));
        outptr[5] = NORMALIZE(DESCALE(tmp12 - tmp1,CONST_BITS+PASS1_BITS+3));
        outptr[3] = NORMALIZE(DESCALE(tmp13 + tmp0,CONST_BITS+PASS1_BITS+3));
        outptr[4] = NORMALIZE(DESCALE(tmp13 - tmp0,CONST_BITS+PASS1_BITS+3));
        
        wsptr += DCTSIZE;		/* advance pointer to next row */
    }
}

#else /* Null implementation */

void idct(pdct_block_t DCAC, pcolor_block_t block)
{
    for(int row = 0 ; row < 8; row ++)
        for(int column = 0; column < 8; column++)
            block[row][column] = DCAC[row][column];
}

#endif

