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
#include "../common/mjpeg423_types.h"
#include "mjpeg423_decoder.h"
#include "../common/util.h"

#define PROFILE_SYSTEM

void mjpeg423_decode(FAT_FILE_HANDLE file_handle, alt_u32* buffer_out,
		hw_video_display* display, int* play_flag, int* skip_fw_flag, int* skip_rv_flag, int* exit_flag, volatile int* timer_flag) {
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

	if (Fat_FileRead(file_handle, file_header, 5 * sizeof(uint32_t)) != 1)
		printf("cannot read input file");
	num_frames = file_header[0];
	w_size = file_header[1];
	h_size = file_header[2];
	num_iframes = file_header[3];
	payload_size = file_header[4];

#ifdef PRINT_TO_CONSOLE
	printf("Decoder start. Num frames #%u\n", num_frames);
	printf("Width %u\n", w_size);
	printf("Height %u\n", h_size);
	printf("Num i frames %u\n", num_iframes);
#endif

	int hCb_size = h_size / 8;           //number of chrominance blocks
	int wCb_size = w_size / 8;
	int hYb_size = h_size / 8; //number of luminance blocks. Same as chrominance in the sample app
	int wYb_size = w_size / 8;

	//trailer structure
	iframe_trailer_t* trailer = malloc(sizeof(iframe_trailer_t) * num_iframes);

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
	if (Fat_FileSeek(file_handle, FILE_SEEK_BEGIN,
			5 * sizeof(uint32_t) + payload_size) == 0)
		printf("cannot seek into file");
	for (int count = 0; count < num_iframes; count++) {
		if (Fat_FileRead(file_handle, &(trailer[count].frame_index),
				sizeof(uint32_t)) == 0)
			printf("cannot read iframe");
		//if(fread(&(trailer[count].frame_index), sizeof(uint32_t), 1, file_in) != 1) error_and_exit("cannot read input file");
		//if(fread(&(trailer[count].frame_position), sizeof(uint32_t), 1, file_in) != 1) error_and_exit("cannot read input file");
		if (Fat_FileRead(file_handle, &(trailer[count].frame_position),
				sizeof(uint32_t)) == 0)
			printf("cannot read iframe");

#ifdef PRINT_TO_CONSOLE
		printf("I frame index %u, ", trailer[count].frame_index);
		printf("position %u\n", trailer[count].frame_position);
#endif
	}
	//set it back to beginning of payload
	if (Fat_FileSeek(file_handle, FILE_SEEK_BEGIN, 5 * sizeof(uint32_t)) == 0)
		printf("cannot seek into file");
	/*if(fseek(file_in,5 * sizeof(uint32_t),SEEK_SET) != 0) error_and_exit("cannot seek into file");
	 */


	//Profiling related variables
	uint file_start_t, file_end_t;
	uint loss_dec_y_t, loss_dec_cb_t, loss_dec_cr_t;
	uint idct_start_t, idct_end_t, idct_y_end_t, idct_block_start_t, idct_block_end_t;
	uint ycbcr_start_t, ycbcr_block_start_t, ycbcr_block_end_t;
	uint display_start_t, display_end_t;


	//encode and write frames
	for (int frame_index = 0; frame_index < num_frames; frame_index++) {
		while(*timer_flag !=1){

		}
		//*timer_flag =0;

		if(*exit_flag != 0){
			*exit_flag = 0;
			printf("\n returning due to exit flag\n");
			return;
		}

		if(*play_flag == 0 && frame_index != 0){
			frame_index--;
			continue;
		}

		if(*skip_fw_flag != 0){
			*skip_fw_flag = 0;
			if(frame_index + 120 >= num_frames){
				printf("\n skipped to end of file \n");
				return;
			} else{
				frame_index = frame_index + 110;
			}
			for (int count = 0; count < num_iframes; count++) {
				if( trailer[count].frame_index >= frame_index){
					frame_index = trailer[count].frame_index;
					if (Fat_FileSeek(file_handle, FILE_SEEK_BEGIN, trailer[count].frame_position) == 0){
						printf("cannot seek to i frame");
					}
					break;
				}
			}
		}
		if(*skip_rv_flag != 0){
			*skip_rv_flag = 0;
			if(frame_index - 120 > 0){
				frame_index = frame_index - 120;
			} else{
				frame_index = 0;
			}
			for (int count = 0; count < num_iframes; count++) {
				if( trailer[count].frame_index >= frame_index){
					if (Fat_FileSeek(file_handle, FILE_SEEK_BEGIN, trailer[count].frame_position) == 0){
						printf("cannot seek to i frame");
					}
					frame_index = trailer[count].frame_index;
					break;
				}
			}

		}

		#ifdef PRINT_TO_CONSOLE
		printf("\nFrame #%u\n",frame_index);
		#endif

		//read frame payload
		//if(fread(frame_header, 4*sizeof(uint32_t), 1, file_in) != 1) error_and_exit("cannot read input file");
		alt_timestamp_start();
		file_start_t = alt_timestamp();
		if (Fat_FileRead(file_handle, frame_header, 4 * sizeof(uint32_t)) != 1)
			printf("cannot read input file");
		frame_size = frame_header[0];
		frame_type = frame_header[1];
		Ysize = frame_header[2];
		Cbsize = frame_header[3];

#ifdef PRINT_TO_CONSOLE
		printf("Frame_size %u\n",frame_size);
		printf("Frame_type %u\n",frame_type);
#endif

		//if(fread(Ybitstream, 1, frame_size - 4 * sizeof(uint32_t), file_in) != (frame_size - 4 * sizeof(uint32_t)))
		if (Fat_FileRead(file_handle, Ybitstream,
				frame_size - 4 * sizeof(uint32_t)) != 1)
			printf("cannot read input file");
		file_end_t = alt_timestamp();
		//error_and_exit("cannot read input file");
		//set the Cb and Cr bitstreams to point to the right location
		Cbbitstream = Ybitstream + Ysize;
		Crbitstream = Cbbitstream + Cbsize;

		loss_dec_y_t = alt_timestamp();
		//lossless decoding
		lossless_decode(hYb_size * wYb_size, Ybitstream, YDCAC, Yquant,
				frame_type);
		loss_dec_cb_t = alt_timestamp();
		lossless_decode(hCb_size * wCb_size, Cbbitstream, CbDCAC, Cquant,
				frame_type);
		loss_dec_cr_t = alt_timestamp();
		lossless_decode(hCb_size * wCb_size, Crbitstream, CrDCAC, Cquant,
				frame_type);

		idct_start_t = alt_timestamp();
		//idct
		for (int b = 0; b < hYb_size * wYb_size; b++)
		{
			idct_block_start_t = alt_timestamp();
			idct(YDCAC[b], Yblock[b]);
			idct_block_end_t = alt_timestamp();
		}
		idct_y_end_t = alt_timestamp();
		for (int b = 0; b < hCb_size * wCb_size; b++)
			idct(CbDCAC[b], Cbblock[b]);
		for (int b = 0; b < hCb_size * wCb_size; b++)
			idct(CrDCAC[b], Crblock[b]);
		idct_end_t = alt_timestamp();

		alt_u32* current_buffer; //size = 4*width*height;
		while (hw_video_display_buffer_is_available(display) != 0) {
		}

		//Add code to update buffer here.

		//Register updated buffer
		//hw_video_display_register_written_buffer(display);
		//ybcbr to rgb conversion
		current_buffer = hw_video_display_get_buffer(display);

		ycbcr_start_t = alt_timestamp();
		for (int h = 0; h < hCb_size; h++)
			for (int w = 0; w < wCb_size; w++) {
				int b = h * wCb_size + w;
				ycbcr_block_start_t = alt_timestamp();
				ycbcr_to_rgb(h << 3, w << 3, w_size, Yblock[b], Cbblock[b],
						Crblock[b], current_buffer);
				ycbcr_block_end_t = alt_timestamp();
				//rgbblock[index] = Cr[y][x] << 16 | Y[y][x] << 8 | Cb[y][x];//= pixel;
			}

//		for(int i = 0; i < 640*480; i++)
//			printf("%X - ", current_buffer[i]);
		//printf("Current buffer: %X\n", current_buffer);
		//memcpy(current_buffer, rgbblock, 480*640*4);
		display_start_t = alt_timestamp();
		hw_video_display_register_written_buffer(display);

		//switch frames
		hw_video_display_switch_frames(display);
		display_end_t = alt_timestamp();

		//frame read, lossless y, lossless cb, lossless cr, idct frame, idct y color, idct block, y2r frame, y2r block, display
		printf("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,\n",frame_type, (file_end_t - file_start_t), (loss_dec_cb_t - loss_dec_y_t), (loss_dec_cr_t - loss_dec_cb_t), (idct_start_t - loss_dec_cr_t), (idct_end_t - idct_start_t), (idct_y_end_t - idct_start_t), (idct_block_end_t-idct_block_start_t), (display_start_t - ycbcr_start_t), (ycbcr_block_end_t - ycbcr_block_start_t), (display_end_t - display_start_t));

#ifdef PRINT_TO_CONSOLE
		printf("output to screen\n");
#endif
		//open and write bmp file

		/*long pos = strlen(filename_out) - 8;      //this assumes the namebase is in the format name0000.bmp
		 filename_out[pos] = (char)(frame_index/1000) + '0';
		 filename_out[pos+1] = (char)(frame_index/100%10) + '0';
		 filename_out[pos+2] = (char)(frame_index/10%10) + '0';
		 filename_out[pos+3] = (char)(frame_index%10) + '0';
		 encode_bmp(rgbblock, w_size, h_size, filename_out);*/
	} //end frame iteration

#ifdef PRINT_TO_CONSOLE
	printf("\nDecoder done.\n");
#endif

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

#ifdef PRINT_TO_CONSOLE
	printf("exit decode");
#endif
}
