//
//  util.h
//  mjpeg423app
//
//  Created by Rodolfo Pellizzoni on 12/24/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef mjpeg423app_util_h
#define mjpeg423app_util_h

#include "mjpeg423_types.h"


/* Debug print macro. Defining DEBUG_JPEG enables the various DEBUG_ macros to print to terminal.
Undefining DEBUG_JPEG disables the debugging prints. */
#define DEBUG_JPEG

#ifdef DEBUG_JPEG
    #define DEBUG_PRINT(str) printf(str);
    #define DEBUG_PRINT_ARG(str,arg) printf(str,arg);
    #define DEBUG_BLOCK(blk) print_block(blk);
    #define DEBUG_DCT(blk) print_dct(blk);
    #define DEBUG_BITSTREAM(num_bits, blk) print_bitstream((num_bits),(blk));
#else
    #define DEBUG_PRINT(str) ;
    #define DEBUG_PRINT_ARG(str,num) ;
    #define DEBUG_BLOCK(blk) ;
    #define DEBUG_DCT(blk) ;
#endif


/* Debug block defines. Defining each of the following macros causes the corresponding processing block
 to be substituted by a "null" block, which does no processing (simply copies input buffer into output buffer */

//#define NULL_QUANT
//#define NULL_LOSSLESS
//#define NULL_DCT
//#define NULL_COLORCONV

//forward declarations for debug functions
void error_and_exit(const char* str);
void print_block(pcolor_block_t b);
void print_dct(pdct_block_t b);
void print_bitstream(int num_bytes, void* bitstream);

#endif
