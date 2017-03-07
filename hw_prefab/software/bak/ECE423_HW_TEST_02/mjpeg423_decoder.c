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
#include "mjpeg423_types.h"
#include "mjpeg423_decoder.h"
#include "util.h"

//declaration. Function implemented in libnsbmp
void encode_bmp(rgb_pixel_t* rgbblock, uint32_t w_size, uint32_t h_size, const char* filename);


//New decoder function to play video

void mjpeg423_decode(FAT_FILE_HANDLE file_handle,
		alt_u32* buffer_out) {
	//header and payload info
	uint32_t num_frames, w_size, h_size, num_iframes, payload_size;
	uint32_t Ysize, Cbsize, frame_size, frame_type;
	uint32_t file_header[5], frame_header[4];

	//file streams
	//FILE* file_in;


	 //file_in = fopen(filename_in, "r");
	  const int nBufferSize = 1000;
	  char* pBuffer = malloc( sizeof(char) * nBufferSize );

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


	h_size = 640;
	w_size = 480;
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
		 printf("I frame index %u, ", trailer[count].frame_index);
		 //printf("position %u\n", trailer[count].frame_position)
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

		 //ybcbr to rgb conversion
		 for (int h = 0; h < hCb_size; h++)
		 for (int w = 0; w < wCb_size; w++) {
		 int b = h * wCb_size + w;
		 ycbcr_to_rgb(h << 3, w << 3, w_size, Yblock[b], Cbblock[b], Crblock[b], rgbblock);
		 }

		 //open and write bmp file

		 /*long pos = strlen(filename_out) - 8;      //this assumes the namebase is in the format name0000.bmp
		 filename_out[pos] = (char)(frame_index/1000) + '0';
		 filename_out[pos+1] = (char)(frame_index/100%10) + '0';
		 filename_out[pos+2] = (char)(frame_index/10%10) + '0';
		 filename_out[pos+3] = (char)(frame_index%10) + '0';
		 encode_bmp(rgbblock, w_size, h_size, filename_out);*/
	 } //end frame iteration
/*
	 DEBUG_PRINT("\nDecoder done.\n\n\n")

	 //close down
	 fclose(file_in);
	 free(rgbblock);
	 free(Yblock);
	 free(Cbblock);
	 free(Crblock);
	 free(YDCAC);
	 free(CbDCAC);
	 free(CrDCAC);
	 free(Ybitstream);
	 free(trailer);*/
	  //Fat_FileClose(file_handle);
	  //printf("Closed the file in decode new");
	 printf("exit decode");
}

//main decoder function
/*void mjpeg423_decode(const char* filename_in, const char* filenamebase_out)
{
    //header and payload info
    uint32_t num_frames, w_size, h_size, num_iframes, payload_size;
    uint32_t Ysize, Cbsize, frame_size, frame_type;
    uint32_t file_header[5], frame_header[4];
    //file streams
    FILE* file_in;
    if((file_in = fopen(filename_in, "r")) == NULL) error_and_exit("cannot open input file");
    char* filename_out = malloc(strlen(filenamebase_out)+1);
    strcpy(filename_out, filenamebase_out);

    //read header
    if(fread(file_header, 5*sizeof(uint32_t), 1, file_in) != 1) error_and_exit("cannot read input file");
    num_frames      = file_header[0];
    w_size          = file_header[1];
    h_size          = file_header[2];
    num_iframes     = file_header[3];
    payload_size    = file_header[4];

    DEBUG_PRINT_ARG("Decoder start. Num frames #%u\n", num_frames)
    DEBUG_PRINT_ARG("Width %u\n", w_size)
    DEBUG_PRINT_ARG("Height %u\n", h_size)
    DEBUG_PRINT_ARG("Num i frames %u\n", num_iframes)

    int hCb_size = h_size/8;           //number of chrominance blocks
    int wCb_size = w_size/8;
    int hYb_size = h_size/8;           //number of luminance blocks. Same as chrominance in the sample app
    int wYb_size = w_size/8;

    //trailer structure
    iframe_trailer_t* trailer = malloc(sizeof(iframe_trailer_t)*num_frames);

    //main data structures. See lab manual for explanation
    rgb_pixel_t* rgbblock;
    if((rgbblock = malloc(w_size*h_size*sizeof(rgb_pixel_t)))==NULL) error_and_exit("cannot allocate rgbblock");
    color_block_t* Yblock;
    if((Yblock = malloc(hYb_size * wYb_size * 64))==NULL) error_and_exit("cannot allocate Yblock");
    color_block_t* Cbblock;
    if((Cbblock = malloc(hCb_size * wCb_size * 64))==NULL) error_and_exit("cannot allocate Cbblock");
    color_block_t* Crblock;
    if((Crblock = malloc(hCb_size * wCb_size * 64))==NULL) error_and_exit("cannot allocate Crblock");;
    dct_block_t* YDCAC;
    if((YDCAC = malloc(hYb_size * wYb_size * 64 * sizeof(DCTELEM)))==NULL) error_and_exit("cannot allocate YDCAC");
    dct_block_t* CbDCAC;
    if((CbDCAC = malloc(hCb_size * wCb_size * 64 * sizeof(DCTELEM)))==NULL) error_and_exit("cannot allocate CbDCAC");
    dct_block_t* CrDCAC;
    if((CrDCAC = malloc(hCb_size * wCb_size * 64 * sizeof(DCTELEM)))==NULL) error_and_exit("cannot allocate CrDCAC");
    //Ybitstream is assigned a size sufficient to hold all bistreams
    //the bitstream is then read from the file into Ybitstream
    //the remaining pointers simply point to the beginning of the Cb and Cr streams within Ybitstream
    uint8_t* Ybitstream;
    if((Ybitstream = malloc(hYb_size * wYb_size * 64 * sizeof(DCTELEM) + 2 * hCb_size * wCb_size * 64 * sizeof(DCTELEM)))==NULL) error_and_exit("cannot allocate bitstream");
    uint8_t* Cbbitstream;
    uint8_t* Crbitstream;

    //read trailer. Note: the trailer information is not used in the sample decoder app
    //set file to beginning of trailer
    if(fseek(file_in, 5 * sizeof(uint32_t) + payload_size, SEEK_SET) != 0) error_and_exit("cannot seek into file");
    for(int count = 0; count < num_iframes; count++){
        if(fread(&(trailer[count].frame_index), sizeof(uint32_t), 1, file_in) != 1) error_and_exit("cannot read input file");
        if(fread(&(trailer[count].frame_position), sizeof(uint32_t), 1, file_in) != 1) error_and_exit("cannot read input file");
        DEBUG_PRINT_ARG("I frame index %u, ", trailer[count].frame_index)
        DEBUG_PRINT_ARG("position %u\n", trailer[count].frame_position)
    }
    //set it back to beginning of payload
    if(fseek(file_in,5 * sizeof(uint32_t),SEEK_SET) != 0) error_and_exit("cannot seek into file");


    //encode and write frames
    for(int frame_index = 0; frame_index < num_frames; frame_index ++){
        DEBUG_PRINT_ARG("\nFrame #%u\n",frame_index)

        //read frame payload
        if(fread(frame_header, 4*sizeof(uint32_t), 1, file_in) != 1) error_and_exit("cannot read input file");
        frame_size  = frame_header[0];
        frame_type  = frame_header[1];
        Ysize       = frame_header[2];
        Cbsize      = frame_header[3];

        DEBUG_PRINT_ARG("Frame_size %u\n",frame_size)
        DEBUG_PRINT_ARG("Frame_type %u\n",frame_type)

        if(fread(Ybitstream, 1, frame_size - 4 * sizeof(uint32_t), file_in) != (frame_size - 4 * sizeof(uint32_t)))
            error_and_exit("cannot read input file");
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

        //ybcbr to rgb conversion
        for (int h = 0; h < hCb_size; h++)
            for (int w = 0; w < wCb_size; w++) {
                int b = h * wCb_size + w;
                ycbcr_to_rgb(h << 3, w << 3, w_size, Yblock[b], Cbblock[b], Crblock[b], rgbblock);
            }

        //open and write bmp file
        long pos = strlen(filename_out) - 8;      //this assumes the namebase is in the format name0000.bmp
        filename_out[pos] = (char)(frame_index/1000) + '0';
        filename_out[pos+1] = (char)(frame_index/100%10) + '0';
        filename_out[pos+2] = (char)(frame_index/10%10) + '0';
        filename_out[pos+3] = (char)(frame_index%10) + '0';
        encode_bmp(rgbblock, w_size, h_size, filename_out);

    } //end frame iteration

    DEBUG_PRINT("\nDecoder done.\n\n\n")

    //close down
    fclose(file_in);
    free(rgbblock);
    free(Yblock);
    free(Cbblock);
    free(Crblock);
    free(YDCAC);
    free(CbDCAC);
    free(CrDCAC);
    free(Ybitstream);
    free(trailer);
}
*/
