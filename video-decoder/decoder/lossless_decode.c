//
//  lossless_encode.c
//  mjpeg423app
//
//  Created by Rodolfo Pellizzoni on 12/23/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include "../common/mjpeg423_types.h"
#include "mjpeg423_decoder.h"
#include "../common/util.h"

#ifndef NULL_LOSSLESS


//Structure to hold a decoded AC or DC coefficient
//bits is the number of decoded bits
typedef struct{
    uint8_t bits;
    uint8_t runlength;
    DCTELEM e;
} huff_input_t;

//forward declarations
void update_buffer(uint32_t* pbitbuffer, void** pbitstream, int* pbitcount, uint8_t size);
huff_input_t input_DC(uint32_t bitbuffer);
huff_input_t input_AC(uint32_t bitbuffer);

/* ********************************
 Lossless decoding block

 The function performs:
 1. runlength decoding based on JPEG zig-zag order
 2. "Variable Length Integer" (VLI) encoding for SIZE, AMPLITUDE

 Each block is encoded using:
 1. (SIZE, AMPLITUDE) for the DC component
 2. (RUNLENGTH, SIZE, AMPLITUDE) for AC

 SIZE and RUNLENGTH are each encoded on 4 bits
 EOB and ZRL follows JPEG specification.

 For I frames, the DC encoding is differential
 (with respect to previous block in the same frame).
 For P frames, all coefficients are encoded differentially
 (with respect to the same coefficient in the previous frame).

 Basic operation:
 uint32_t bitbuffer is used as a decoding buffer.
 The next DC or AC coefficient is contained in the MSB of bitbuffer.
 Every time we decode a DC or AC coefficient, the corresponding
 function returns the number of extracted bits.
 Then update_buffer updates bitbuffer by removing the extracted bits
 and "shifting in" the same number of bits from the bitstream.
 To perform the update, the bitstream pointer is incremented every
 time bits from a new byte in the bistream are shifted in.

******************************** */
void lossless_decode(int num_blocks, void* bitstream, dct_block_t* DCACq, dct_block_t quant, bool P)
{
    //bitbuffer
    uint32_t bitbuffer = 0;
    //bit position in the bitstream pointed to by bitstream.
    //I.e., if bitcount = 3, then 3 bits from the byte
    //pointed to by bitstream have already been shifted in bitbuffer
    int bitcount = 0;

    //shift in the first 32 bits
    update_buffer(&bitbuffer, &bitstream, &bitcount, 32);

    huff_input_t ib;
    //Used for I frame DC differential encoding
    DCTELEM cur = 0;

    if(P == 0)
        memset(DCACq, 0, num_blocks*64*sizeof(DCTELEM));

    for(int count = 0; count < num_blocks; count ++){
        //pe is used to write the DCT coefficients in zig-zag order
        DCTELEM* pe = (DCTELEM*)(DCACq[count]);

        //decode DC coefficients
        ib = input_DC(bitbuffer);
        update_buffer(&bitbuffer, &bitstream, &bitcount, ib.bits);
#ifndef NULL_QUANT
        if(P){ //differential decoding based on previous frame
            pe[0] += (ib.e * (((DCTELEM*)quant)[0]) );
        }
        else{ //differential decoding based on previous block
            cur += ib.e;
            pe[0] = cur * (((DCTELEM*)quant)[0]);
        }
#else   //null dequantization, no differential
        pe[0] = ib.e;
#endif

        uint8_t index = 1; //zig-zag order index for AC values in the 8x8 matrix, [0, 63]
        while(1){
            //decode AC coefficient
            ib = input_AC(bitbuffer);
            update_buffer(&bitbuffer, &bitstream, &bitcount, ib.bits);
            if(ib.e == 0){
                if(ib.runlength == 15){
                    //EZL
                    index += 16;
                }
                else {
                    //END
                    break; //break from while and go to next block
                }
            }
            else{ //ib.e != 0

                index += ib.runlength;
                //now add the actual value
#ifndef NULL_QUANT
                if(P){ //differential decoding based on previous frame
                    pe[zigzag_table[index]] += ib.e * (((DCTELEM*)quant)[zigzag_table[index]]);
                }
                else{ //no differential decoding
                    pe[zigzag_table[index]] = ib.e * (((DCTELEM*)quant)[zigzag_table[index]]);
                }
#else
                pe[zigzag_table[index]] = ib.e;
#endif
                if(index >= 63) break;
                index++;
            }
        }
    }
}


//update buffer
void update_buffer(uint32_t* pbitbuffer, void** pbitstream, int* pbitcount,
        uint8_t size) {
    //remove decoded bits
    (*pbitbuffer) <<= size;
    //total number of bits to be shifted in
    *pbitcount += size;
    if (*pbitcount >= 8) { //we need to shift in at least 1 byte
        *pbitbuffer |= *((uint8_t*) (*pbitstream)) << (*pbitcount - 8);
        *pbitstream = ((uint8_t*) *pbitstream) + 1;
        if (*pbitcount >= 16) { //at least 2 bytes
            *pbitbuffer |= *((uint8_t*) (*pbitstream)) << (*pbitcount - 16);
            *pbitstream = ((uint8_t*) *pbitstream) + 1;
            if (*pbitcount >= 24) { //at least 3 bytes
                *pbitbuffer |= *((uint8_t*) (*pbitstream)) << (*pbitcount - 24);
                *pbitstream = ((uint8_t*) *pbitstream) + 1;
                if (*pbitcount == 32) { //all 4 bytes shifted in
                    *pbitbuffer |= *((uint8_t*) (*pbitstream));
                    *pbitstream = ((uint8_t*) *pbitstream) + 1;
                }
            }
        }
    }
    *pbitcount &= 7; //the resulting *pbitcount should be between 0 and 7
}
// -----------------------------------------------------------
//VLI decoding.
//x is encoded amplitude (two's complement), s is size.
//Returns the decoded amplitude in two's complement. Notice x cannot be zero.
#define HUFF_EXTEND(x,s)  ((x) < (1<<((s)-1)) ? (x) + (((-1)<<(s)) + 1) : (x))

//extract num bits from the buffer and returns them
#define INPUT_BITS(buffer, num) (buffer) >> (32 - (num))

//DC decode function
huff_input_t input_DC(uint32_t bitbuffer)
{
    huff_input_t ib;
    uint8_t size = INPUT_BITS(bitbuffer, 4);
    if(size == 0){
        ib.e = 0;
        ib.bits = 4;
    }
    else{
        bitbuffer <<= 4;
        ib.e = HUFF_EXTEND(INPUT_BITS(bitbuffer,size),size);
        ib.bits = size + 4;
    }
    return ib;
}

//AC decode function
huff_input_t input_AC(uint32_t bitbuffer)
{
    huff_input_t ib;
    uint8_t size;
    ib.runlength  = INPUT_BITS(bitbuffer, 4);
    bitbuffer <<= 4;
    size  = INPUT_BITS(bitbuffer, 4);
    if(size == 0) {
        //return a value of 0. Notice this is ok size if size == 0, then it's either a END or ZRL,
        //and if size > 0, the amplitude cannot be 0.
        ib.e = 0;
        ib.bits = 8;
    }
    else{
        bitbuffer <<= 4;
        ib.e = HUFF_EXTEND(INPUT_BITS(bitbuffer,size),size);
        ib.bits = size + 8;
    }
    return ib;
}


#else /* Null lossless decoding */

void lossless_decode(int num_blocks, void* bitstream, dct_block_t* DCACq, dct_block_t quant, bool P)
{
    uint32_t len = 0;
    DCTELEM cur = 0;
    for(int blocks = 0; blocks < num_blocks; blocks++){
#ifndef NULL_QUANT
        if(P){
            ((DCTELEM*)(DCACq[blocks]))[0] += ((DCTELEM*)bitstream)[len++] * ((DCTELEM*)quant)[0];
        }
        else{
            cur += ((DCTELEM*)bitstream)[len++];
            ((DCTELEM*)(DCACq[blocks]))[0] = cur * (((DCTELEM*)quant)[0]);
        }
        for(int count = 1; count < 64; count ++){
            if(P)
                ((DCTELEM*)(DCACq[blocks]))[count] += ((DCTELEM*)bitstream)[len++] * ((DCTELEM*)quant)[count];
            else
                ((DCTELEM*)(DCACq[blocks]))[count] = ((DCTELEM*)bitstream)[len++] * ((DCTELEM*)quant)[count] ;
        }
#else
        for(int count = 0; count < 64; count ++)
            ((DCTELEM*)(DCACq[blocks]))[count] = ((DCTELEM*)bitstream)[len++];

#endif
    }
}

#endif

