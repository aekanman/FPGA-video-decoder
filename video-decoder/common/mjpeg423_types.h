//
//  mjpeg423_types.h
//  mjpeg423app
//
//  Created by Rodolfo Pellizzoni on 12/23/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef mjpeg423app_mjpeg423_types_h
#define mjpeg423app_mjpeg423_types_h

#include <stdlib.h>
#include <stdint.h>

#ifndef bool
typedef int bool;
#define FALSE (0)
#define TRUE !FALSE
#endif

//trailer structure
typedef struct {
    uint32_t frame_index;
    uint32_t frame_position;
} iframe_trailer_t;

//8x8 blocks for Y'CbCr color components on 8 bits
//since C is quite peculiar about dimensional matrixes, 
//we need to declare a typedef for the block and one for the pointer to the block
//so:   color_block_t is a 2-D matrix
//      pcolor_block_t is a pointer to a color_block
//      color_block_t* is an array of color blocks
typedef uint8_t color_block_t[8][8];
typedef uint8_t (*pcolor_block_t)[8];

//type for DCT elements. 
//With 8 bit color components, 16 bits are sufficient to hold the DCT values
//Declaring it as 32 bits would use more space, but might save on some 16 -> 32 integer conversions.
#define DCTELEM int16_t
//#define DCTELEM int32_t
//8x8 blocks for DCT elements
typedef DCTELEM dct_block_t[8][8];
typedef DCTELEM (*pdct_block_t)[8];

//We assume that right shift corresponds to signed division by 2 with
//rounding towards minus infinity.  This is correct for typical "arithmetic
//shift" instructions that shift in copies of the sign bit.  But some
//C compilers implement >> with an unsigned shift.  For these machines you
//must define RIGHT_SHIFT_IS_UNSIGNED.
//Note: you do not need to define it for Nios II
//#define RIGHT_SHIFT_IS_UNSIGNED

//RGB structure
//Note: alpha channel is not used in the project, but it ensures the structure is 32-bits aligned
//This is the same byte ordering used in bmp
typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;      
} rgb_pixel_t;

//tables
extern dct_block_t Yquant;
extern dct_block_t Cquant;
extern int zigzag_table[64];

#endif
