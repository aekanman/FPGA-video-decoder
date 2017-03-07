//
//  mjpeg423_decoder.c
//  mjpeg423app
//
//  Created by Rodolfo Pellizzoni on 12/24/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "mjpeg423_types.h"
#include "mjpeg423_decoder.h"
#include "common/util.h"
#include "common/dct_math.h"


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

#define NULL_COLORCONV
#ifndef NULL_COLORCONV

//normalize result between 0 and 255
//this is required to handle precision errors that might cause the obtained color to fall out of range
#define NORMALIZE_RGB(x) (temp = (x), ( (temp < 0) ? 0 : ( temp = (temp >> 14), (temp > 255) ? 255 : temp  ) ) )


#else

//null implementation
void ycbcr_to_rgb(int h, int w, uint32_t w_size, pcolor_block_t Y, pcolor_block_t Cb, pcolor_block_t Cr, alt_u32* rgbblock)
{
    int index;
    for (int y = 0; y < 8; y++){
        index = (h+y) * w_size + w;
        for(int x = 0; x < 8; x++){
            rgb_pixel_t pixel;
            pixel.alpha = 0;
            pixel.red = Cr[y][x];
            pixel.green = Y[y][x];
            pixel.blue = Cb[y][x];
            rgbblock[index] = Cr[y][x] << 16 | Y[y][x] << 8 | Cb[y][x];//= pixel;
            index++;
        }
    }
}

#endif


//New decoder function to play video

void mjpeg423_decode(FAT_FILE_HANDLE file_handle,
		alt_u32* buffer_out, hw_video_display* display) {
	//header and payload info
	uint32_t num_frames, w_size, h_size, num_iframes, payload_size;
	uint32_t Ysize, Cbsize, frame_size, frame_type;
	uint32_t file_header[5], frame_header[4];

	//file streams
	//FILE* file_in;


	 //file_in = fopen(filename_in, "r");

	  //if (Fat_FileRead(file_handle, pBuffer, nBufferSize) != 0){
	//	printf("Opened the file in decode new");
	  //}

	 //read header
	 if(Fat_FileRead(file_handle, file_header, 5*sizeof(uint32_t))!= 1) printf("cannot read input file");
	 num_frames      = file_header[0];
	 w_size          = file_header[1];
	 h_size          = file_header[2];
	 num_iframes     = file_header[3];
	 payload_size    = file_header[4];

	 printf("Decoder start. Num frames #%u\n", num_frames);
	 printf("Width %u\n", w_size);
	 printf("Height %u\n", h_size);
	 printf("Num i frames %u\n", num_iframes);

	int hCb_size = h_size / 8;           //number of chrominance blocks
	int wCb_size = w_size / 8;
	int hYb_size = h_size / 8; //number of luminance blocks. Same as chrominance in the sample app
	int wYb_size = w_size / 8;

	//trailer structure
	iframe_trailer_t* trailer = malloc(sizeof(iframe_trailer_t) * num_frames);

	//main data structures. See lab manual for explanation
	rgb_pixel_t* rgbblock;
	if ((rgbblock = malloc(w_size * h_size * sizeof(rgb_pixel_t))) == NULL)
		printf("cannot allocate rgbblock");
	color_block_t* Yblock;
	if ((Yblock = malloc(hYb_size * wYb_size * 64)) == NULL)
		printf("cannot allocate Yblock");
	color_block_t* Cbblock;
	if ((Cbblock = malloc(hCb_size * wCb_size * 64)) == NULL)
		printf("cannot allocate Cbblock");
	color_block_t* Crblock;
	if ((Crblock = malloc(hCb_size * wCb_size * 64)) == NULL)
		printf("cannot allocate Crblock");
	;
	dct_block_t* YDCAC;
	if ((YDCAC = malloc(hYb_size * wYb_size * 64 * sizeof(DCTELEM))) == NULL)
		printf("cannot allocate YDCAC");
	dct_block_t* CbDCAC;
	if ((CbDCAC = malloc(hCb_size * wCb_size * 64 * sizeof(DCTELEM))) == NULL)
		printf("cannot allocate CbDCAC");
	dct_block_t* CrDCAC;
	if ((CrDCAC = malloc(hCb_size * wCb_size * 64 * sizeof(DCTELEM))) == NULL)
		printf("cannot allocate CrDCAC");
	//Ybitstream is assigned a size sufficient to hold all bistreams
	//the bitstream is then read from the file into Ybitstream
	//the remaining pointers simply point to the beginning of the Cb and Cr streams within Ybitstream
	uint8_t* Ybitstream;
	if ((Ybitstream = malloc(
			hYb_size * wYb_size * 64 * sizeof(DCTELEM)
					+ 2 * hCb_size * wCb_size * 64 * sizeof(DCTELEM))) == NULL)
		printf("cannot allocate bitstream");
	uint8_t* Cbbitstream;
	uint8_t* Crbitstream;

	//read trailer. Note: the trailer information is not used in the sample decoder app
	//set file to beginning of trailer
	//Fat_FileSeek(file_handle, 5 * sizeof(uint32_t) + payload_size, 0)
	 if(Fat_FileSeek(file_handle, FILE_SEEK_BEGIN, 5 * sizeof(uint32_t) + payload_size) == 0) printf("cannot seek into file");
	 for(int count = 0; count < num_iframes; count++){
		 if(Fat_FileRead(file_handle, &(trailer[count].frame_index), sizeof(uint32_t)) == 0) printf("cannot read iframe");
		 //if(fread(&(trailer[count].frame_index), sizeof(uint32_t), 1, file_in) != 1) error_and_exit("cannot read input file");
		 //if(fread(&(trailer[count].frame_position), sizeof(uint32_t), 1, file_in) != 1) error_and_exit("cannot read input file");
		 if(Fat_FileRead(file_handle, &(trailer[count].frame_position), sizeof(uint32_t)) == 0) printf("cannot read iframe");
		 printf("I frame index %u, ", trailer[count].frame_index);
		 printf("position %u\n", trailer[count].frame_position);
	 }
	 //set it back to beginning of payload
	 if(Fat_FileSeek(file_handle, FILE_SEEK_BEGIN, 5 * sizeof(uint32_t)) == 0) printf("cannot seek into file");
	 /*if(fseek(file_in,5 * sizeof(uint32_t),SEEK_SET) != 0) error_and_exit("cannot seek into file");
	 */

	 //encode and write frames
	 for(int frame_index = 0; frame_index < num_frames; frame_index ++){
		 printf("\nFrame #%u\n",frame_index);

		 //read frame payload
		 //if(fread(frame_header, 4*sizeof(uint32_t), 1, file_in) != 1) error_and_exit("cannot read input file");
		 if(Fat_FileRead(file_handle, frame_header, 4*sizeof(uint32_t))!= 1) printf("cannot read input file");
		 frame_size  = frame_header[0];
		 frame_type  = frame_header[1];
		 Ysize       = frame_header[2];
		 Cbsize      = frame_header[3];

		 printf("Frame_size %u\n",frame_size);
		 printf("Frame_type %u\n",frame_type);

		 //if(fread(Ybitstream, 1, frame_size - 4 * sizeof(uint32_t), file_in) != (frame_size - 4 * sizeof(uint32_t)))
		 if(Fat_FileRead(file_handle, Ybitstream, frame_size - 4 * sizeof(uint32_t))!= 1) printf("cannot read input file");
		 //error_and_exit("cannot read input file");
		 //set the Cb and Cr bitstreams to point to the right location
		 Cbbitstream = Ybitstream + Ysize;
		 Crbitstream = Cbbitstream + Cbsize;

		 //lossless decoding
		 lossless_decode(hYb_size*wYb_size, Ybitstream, YDCAC, Yquant, frame_type);
		 lossless_decode(hCb_size*wCb_size, Cbbitstream, CbDCAC, Cquant, frame_type);
		 lossless_decode(hCb_size*wCb_size, Crbitstream, CrDCAC, Cquant, frame_type);

		 //fdct
		 for(int b = 0; b < hYb_size*wYb_size; b++) idct(YDCAC[b], Yblock[b]);
		 for(int b = 0; b < hCb_size*wCb_size; b++) idct(CbDCAC[b], Cbblock[b]);
		 for(int b = 0; b < hCb_size*wCb_size; b++) idct(CrDCAC[b], Crblock[b]);

		 alt_u32* current_buffer; //size = 4*width*height;
		 while (hw_video_display_buffer_is_available(display) != 0) {}

		//Add code to update buffer here.

		//Register updated buffer
		//hw_video_display_register_written_buffer(display);
		 //ybcbr to rgb conversion
		 current_buffer = hw_video_display_get_buffer(display);

		 for (int h = 0; h < hCb_size; h++)
			 for (int w = 0; w < wCb_size; w++) {
				 int b = h * wCb_size + w;
				 //printf("\n%x \n", Yblock[b][0][0]);
				 ycbcr_to_rgb(h << 3, w << 3, w_size, Yblock[b], Cbblock[b], Crblock[b], current_buffer);
				 //rgbblock[index] = Cr[y][x] << 16 | Y[y][x] << 8 | Cb[y][x];//= pixel;
			 }

//		for(int i = 0; i < 640*480; i++)
//			printf("%X - ", current_buffer[i]);
		//printf("Current buffer: %X\n", current_buffer);
		//memcpy(current_buffer, rgbblock, 480*640*4);
		hw_video_display_register_written_buffer(display);

		//switch frames
		hw_video_display_switch_frames(display);




		 printf("output to screen\n");
		 //open and write bmp file

		 /*long pos = strlen(filename_out) - 8;      //this assumes the namebase is in the format name0000.bmp
		 filename_out[pos] = (char)(frame_index/1000) + '0';
		 filename_out[pos+1] = (char)(frame_index/100%10) + '0';
		 filename_out[pos+2] = (char)(frame_index/10%10) + '0';
		 filename_out[pos+3] = (char)(frame_index%10) + '0';
		 encode_bmp(rgbblock, w_size, h_size, filename_out);*/
	 } //end frame iteration

	 printf("\nDecoder done.\n\n\n");

	 //close down
	 //fclose(file_in);
	 Fat_FileClose(file_handle);
	 free(rgbblock);
	 free(Yblock);
	 free(Cbblock);
	 free(Crblock);
	 free(YDCAC);
	 free(CbDCAC);
	 free(CrDCAC);
	 free(Ybitstream);
	 free(trailer);

	 printf("exit decode");
}
