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
#include "system.h"

#include "../common/util.h"

#define PROFILE_SYSTEM
#define ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK    (1 << 14)
#define ALTERA_MSGDMA_CSR_GLOBAL_INTERRUPT_MASK               (1 << 4)

const char *read_dma_name = READ_DMA_0_CSR_NAME;
const char *write_dma_name = WRITE_DMA_0_CSR_NAME;
alt_u32 status;
alt_u32 timeout = 0;
alt_u32 message[2];
typedef struct{
	int num_blocks;
	uint8_t* bitstream;
	dct_block_t* DCACq;
	uint32_t frame_type;
} ld_mailbox_t;

typedef struct{
	color_block_t* Yblock;
	color_block_t* Cbblock;
	color_block_t* Crblock;
	alt_u32* current_buffer;
	uint32_t w_size;
	uint32_t h_size;
} cc_mailbox_t;

alt_u32* last_buffer; //size = 4*width*height;

alt_u32 odd_frame = 0; //size = 4*width*height;

int idct_hw(pdct_block_t DCAC, pcolor_block_t block, alt_msgdma_dev *read_pointer, alt_msgdma_dev *write_pointer);
int idct_hw_frame(pdct_block_t* DCAC, pcolor_block_t* block, int block_count, alt_msgdma_dev *read_pointer, alt_msgdma_dev *write_pointer);

int idct_test(alt_msgdma_dev *read_pointer, alt_msgdma_dev *write_pointer);\
int buffer_was_full;
volatile int dma_complete = 1;

//Interrupt related function prototypes
static void write_dma_isr(void* context);

void mjpeg423_decode(FAT_FILE_HANDLE file_handle, HW_video_display* display, int* skip_fw_flag,
		int* skip_rv_flag, int* exit_flag, altera_avalon_mailbox_dev* mailbox_sender,
		altera_avalon_mailbox_dev* mailbox_sender3, altera_avalon_mailbox_dev* mailbox_rcv,
		altera_avalon_mailbox_dev* mailbox_rcv3)
{
	//header and payload info
	uint32_t num_frames, w_size, h_size, num_iframes, payload_size;
	uint32_t Ysize, Cbsize, frame_size, frame_type;
	uint32_t file_header[5], frame_header[4];

	ld_mailbox_t* Cb_msg;
	ld_mailbox_t* Y_msg;
	cc_mailbox_t* CPU1_CC_msg;
	cc_mailbox_t* CPU2_CC_msg;
	Cb_msg = malloc(sizeof(ld_mailbox_t));
	Y_msg = malloc(sizeof(ld_mailbox_t));
	CPU1_CC_msg = malloc(sizeof(cc_mailbox_t));
	CPU2_CC_msg = malloc(sizeof(cc_mailbox_t));
	//IDCT hardware setup
	//open dma streams
	alt_msgdma_dev *read_device_ptr = alt_msgdma_open(read_dma_name);
	alt_msgdma_dev *write_device_ptr = alt_msgdma_open(write_dma_name);
	//register callback
	alt_msgdma_register_callback(read_device_ptr, write_dma_isr,
			ALTERA_MSGDMA_CSR_GLOBAL_INTERRUPT_MASK, NULL);
	alt_msgdma_register_callback(write_device_ptr, write_dma_isr,
			ALTERA_MSGDMA_CSR_GLOBAL_INTERRUPT_MASK, NULL);

	//idct_test(read_device_ptr, write_device_ptr);

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


	color_block_t* Yblock2;
	if ((Yblock2 = malloc(hYb_size * wYb_size * 64)) == NULL)
		printf("cannot allocate Yblock");
	color_block_t* Cbblock2;
	if ((Cbblock2 = malloc(hCb_size * wCb_size * 64)) == NULL)
		printf("cannot allocate Cbblock");
	color_block_t* Crblock2;
	if ((Crblock2 = malloc(hCb_size * wCb_size * 64)) == NULL)
		printf("cannot allocate Crblock");


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
	alt_u32 idct_dma_start, idct_dma_end, cpu2_lossless_start,cpu2_lossless_end, cpu2_cc_start, cpu2_cc_end, cc_time;
	//DMA constructors
	alt_msgdma_standard_descriptor mm_to_st_dma_struct_Y;
	alt_msgdma_standard_descriptor st_to_mm_dma_struct_Y;
	alt_msgdma_standard_descriptor mm_to_st_dma_struct_Cb;
	alt_msgdma_standard_descriptor st_to_mm_dma_struct_Cb;
	alt_msgdma_standard_descriptor mm_to_st_dma_struct_Cr;
	alt_msgdma_standard_descriptor st_to_mm_dma_struct_Cr;


	alt_msgdma_standard_descriptor st_to_mm_dma_struct_Y2;
	alt_msgdma_standard_descriptor st_to_mm_dma_struct_Cb2;
	alt_msgdma_standard_descriptor st_to_mm_dma_struct_Cr2;



	if (0 != alt_msgdma_construct_standard_mm_to_st_descriptor(
			read_device_ptr, &mm_to_st_dma_struct_Y, YDCAC,
			hYb_size * wYb_size * 64 * 2,
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor mm to st\n");
	}
	if (0 != alt_msgdma_construct_standard_st_to_mm_descriptor(
			write_device_ptr, &st_to_mm_dma_struct_Y, Yblock,
			hYb_size * wYb_size * 64,
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor st to mm\n");
	}
	if (0 != alt_msgdma_construct_standard_mm_to_st_descriptor(
			read_device_ptr, &mm_to_st_dma_struct_Cb, CbDCAC,
			hCb_size * wCb_size * 64 * 2,
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor mm to st\n");
	}
	if (0 != alt_msgdma_construct_standard_st_to_mm_descriptor(
			write_device_ptr, &st_to_mm_dma_struct_Cb, Cbblock,
			hCb_size * wCb_size * 64,
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor st to mm\n");
	}
	if (0 != alt_msgdma_construct_standard_mm_to_st_descriptor(
			read_device_ptr, &mm_to_st_dma_struct_Cr, CrDCAC,
			hCb_size * wCb_size * 64 * 2,
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor mm to st\n");
	}
	if (0 != alt_msgdma_construct_standard_st_to_mm_descriptor(
			write_device_ptr, &st_to_mm_dma_struct_Cr, Crblock,
			hCb_size * wCb_size * 64,
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor st to mm\n");
	}



	if (0 != alt_msgdma_construct_standard_st_to_mm_descriptor(
			write_device_ptr, &st_to_mm_dma_struct_Y2, Yblock2,
			hYb_size * wYb_size * 64,
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor st to mm\n");
	}

	if (0 != alt_msgdma_construct_standard_st_to_mm_descriptor(
			write_device_ptr, &st_to_mm_dma_struct_Cb2, Cbblock2,
			hCb_size * wCb_size * 64,
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor st to mm\n");
	}

	if (0 != alt_msgdma_construct_standard_st_to_mm_descriptor(
			write_device_ptr, &st_to_mm_dma_struct_Cr2, Crblock2,
			hCb_size * wCb_size * 64,
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor st to mm\n");
	}

	alt_timestamp_start();

	//encode and write frames
	//for (int frame_index = 0; frame_index < 10; frame_index++) {
	for (int frame_index = 0; frame_index < num_frames; frame_index++) {


		//#ifdef FIXED_FRAMERATE
		//while(*timer_flag !=1){
		//}
		//*timer_flag =0;
		//#endif

		if(*exit_flag != 0){
			//*exit_flag = 0;
			//printf("\n returning due to exit flag\n");
			//return;
		}

		//if(*start_decode_flag == 0 && frame_index != 0){
		//	frame_index--;
		//	continue;
		//}
//		alt_dcache_flush_all(); //added by aaron

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
//		alt_dcache_flush_all(); //added by aaron
		//read frame payload
		//if(fread(frame_header, 4*sizeof(uint32_t), 1, file_in) != 1) error_and_exit("cannot read input file");
		//alt_timestamp_start();
		//file_start_t = alt_timestamp();
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
		//file_end_t = alt_timestamp();
		//error_and_exit("cannot read input file");
		//set the Cb and Cr bitstreams to point to the right location
		Cbbitstream = Ybitstream + Ysize;
		Crbitstream = Cbbitstream + Cbsize;

		if (frame_index>=2){
			altera_avalon_mailbox_retrieve_poll (mailbox_rcv3,message, timeout);
			altera_avalon_mailbox_retrieve_poll (mailbox_rcv,message, timeout);
			cpu2_cc_end = alt_timestamp();
			HW_video_display_register_written_buffer(display);

		}




		Cb_msg->DCACq = CbDCAC;
		Cb_msg->bitstream = Cbbitstream;
		Cb_msg->frame_type = frame_type;
		Cb_msg->num_blocks = hCb_size * wCb_size;
		Y_msg->DCACq = YDCAC;
		Y_msg->bitstream = Ybitstream;
		Y_msg->frame_type = frame_type;
		Y_msg->num_blocks = hYb_size * wYb_size;
		alt_u32 CPU1_message[2];
		alt_u32 CPU2_message[2];
		CPU1_message[0]= 1;
		CPU1_message[1]=Y_msg;
		CPU2_message[0]= 1;
		CPU2_message[1]=Cb_msg;
		alt_dcache_flush_all();




		//  printf ("bitstream %d, DCACq %d, frame_type  %d \n",Cr_msg->bitstream,Cr_msg->DCACq,Cr_msg->frame_type);


		status = altera_avalon_mailbox_send(mailbox_sender3,CPU2_message,timeout,POLL);
		cpu2_lossless_start = alt_timestamp();
		status = altera_avalon_mailbox_send(mailbox_sender,CPU1_message,timeout,POLL);

		if (odd_frame == 0){
			CPU1_CC_msg->Cbblock = Cbblock2;
			CPU1_CC_msg->Crblock =  Crblock2;
			CPU1_CC_msg->Yblock = Yblock2;
			CPU1_CC_msg->current_buffer= last_buffer;
			CPU1_CC_msg->h_size = h_size;
			CPU1_CC_msg->w_size = w_size;

			CPU2_CC_msg->Cbblock = Cbblock2;
			CPU2_CC_msg->Crblock=  Crblock2;
			CPU2_CC_msg->Yblock= Yblock2;
			CPU2_CC_msg->current_buffer= last_buffer;
			CPU2_CC_msg->h_size = h_size;
			CPU2_CC_msg->w_size = w_size;
		}

		if (odd_frame == 1){
			CPU1_CC_msg->Cbblock = Cbblock;
			CPU1_CC_msg->Crblock =  Crblock;
			CPU1_CC_msg->Yblock = Yblock;
			CPU1_CC_msg->current_buffer= last_buffer;
			CPU1_CC_msg->h_size = h_size;
			CPU1_CC_msg->w_size = w_size;

			CPU2_CC_msg->Cbblock = Cbblock;
			CPU2_CC_msg->Crblock=  Crblock;
			CPU2_CC_msg->Yblock= Yblock;
			CPU2_CC_msg->current_buffer= last_buffer;
			CPU2_CC_msg->h_size = h_size;
			CPU2_CC_msg->w_size = w_size;
		}

		CPU1_message[0]= 2;
		CPU1_message[1]=CPU1_CC_msg;
		CPU2_message[0]= 2;
		CPU2_message[1]=CPU2_CC_msg;

		//				alt_dcache_flush_all();

		//	loss_dec_y_t = alt_timestamp();
		//lossless decoding
		//	lossless_decode(hYb_size * wYb_size, Ybitstream, YDCAC, Yquant,
		//			frame_type);

		alt_dcache_flush_all();

		lossless_decode(hCb_size * wCb_size, Crbitstream, CrDCAC, Cquant,
				frame_type);
		//alt_dcache_flush_all(); //added by aaron

		while(dma_complete == 0);
		dma_complete = 0;
		idct_dma_start = alt_timestamp();
		if(odd_frame == 0 ){
			if (0 != alt_msgdma_standard_descriptor_async_transfer(write_device_ptr,
					&st_to_mm_dma_struct_Cr)) {
				printf("failed starting transfer st to mm\n");
			}
		}

		if(odd_frame == 1 ){
			if (0 != alt_msgdma_standard_descriptor_async_transfer(write_device_ptr,
					&st_to_mm_dma_struct_Cr2)) {
				printf("failed starting transfer st to mm\n");
			}
		}



		if (0 != alt_msgdma_standard_descriptor_async_transfer(read_device_ptr,
				&mm_to_st_dma_struct_Cr)) {
			printf("failed starting transfer mm to st\n");
		}
		idct_dma_end = alt_timestamp();
//		alt_dcache_flush_all();

		//alt_dcache_flush_all(); //added by aaron
		if (frame_index!=0){

			if (odd_frame == 1){
				for (int h = (8*hCb_size/10); h < (9*hCb_size/10); h++)
					for (int w = 0; w < wCb_size; w++) {
						int b = h * wCb_size + w;
						//	ycbcr_block_start_t = alt_timestamp();
						ycbcr_to_rgb(h << 3, w << 3, w_size, Yblock[b], Cbblock[b],
								Crblock[b], last_buffer);
						//	ycbcr_block_end_t = alt_timestamp();
						//rgbblock[index] = Cr[y][x] << 16 | Y[y][x] << 8 | Cb[y][x];//= pixel;
					}
			}

			if (odd_frame == 0){
				for (int h = (8*hCb_size/10); h < (9*hCb_size/10); h++)
					for (int w = 0; w < wCb_size; w++) {
						int b = h * wCb_size + w;
						//		ycbcr_block_start_t = alt_timestamp();
						ycbcr_to_rgb(h << 3, w << 3, w_size, Yblock2[b], Cbblock2[b],
								Crblock2[b], last_buffer);
						//		ycbcr_block_end_t = alt_timestamp();
						//rgbblock[index] = Cr[y][x] << 16 | Y[y][x] << 8 | Cb[y][x];//= pixel;
					}
			}

		}

		//idct_hw_frame_async(YDCAC, Yblock, hYb_size * wYb_size, read_device_ptr, write_device_ptr);

		//	loss_dec_cb_t = alt_timestamp();



		//lossless_decode(hCb_size * wCb_size, Cbbitstream, CbDCAC, Cquant,
		//		frame_type);
		alt_dcache_flush_all();
		altera_avalon_mailbox_retrieve_poll (mailbox_rcv3,message, timeout);
		cpu2_lossless_end = alt_timestamp();
		if(frame_index !=0){
			if(frame_index != 9)
			{
				cc_time = cpu2_cc_end - cpu2_cc_start;
			cpu2_cc_start = alt_timestamp();

			}
			status = altera_avalon_mailbox_send(mailbox_sender3,CPU2_message,timeout,POLL);
		}

		while(dma_complete == 0);
		dma_complete = 0;
		//alt_dcache_flush_all(); //added by aaron
		if(odd_frame == 0){
			if (0 != alt_msgdma_standard_descriptor_async_transfer(write_device_ptr,
					&st_to_mm_dma_struct_Cb)) {
				printf("failed starting transfer st to mm\n");
			}
		}

		if(odd_frame == 1 ){
			if (0 != alt_msgdma_standard_descriptor_async_transfer(write_device_ptr,
					&st_to_mm_dma_struct_Cb2)) {
				printf("failed starting transfer st to mm\n");
			}
		}
		if (0 != alt_msgdma_standard_descriptor_async_transfer(read_device_ptr,
				&mm_to_st_dma_struct_Cb)) {
			printf("failed starting transfer mm to st\n");
		}

		//idct_hw_frame(CbDCAC, Cbblock, hCb_size * wCb_size, read_device_ptr, write_device_ptr);

		//loss_dec_cr_t = alt_timestamp();
		altera_avalon_mailbox_retrieve_poll (mailbox_rcv,message, timeout);
		// lossless_decode(hCb_size * wCb_size, Crbitstream, CrDCAC, Cquant,
		//		frame_type);
		//idct_hw_frame(CrDCAC, Crblock, hCb_size * wCb_size, read_device_ptr, write_device_ptr);
		//printf("Decoded IDCT for one frame.\n");
		alt_dcache_flush_all();
		if(frame_index !=0){
			status = altera_avalon_mailbox_send(mailbox_sender,CPU1_message,timeout,POLL);
		}

		while(dma_complete == 0);
		dma_complete = 0;
		if(odd_frame == 0){
			if (0 != alt_msgdma_standard_descriptor_async_transfer(write_device_ptr,
					&st_to_mm_dma_struct_Y)) {
				printf("failed starting transfer st to mm\n");
			}
		}

		if(odd_frame == 1 ){
			if (0 != alt_msgdma_standard_descriptor_async_transfer(write_device_ptr,
					&st_to_mm_dma_struct_Y2)) {
				printf("failed starting transfer st to mm\n");
			}
		}

		if (0 != alt_msgdma_standard_descriptor_async_transfer(read_device_ptr,
				&mm_to_st_dma_struct_Y)) {
			printf("failed starting transfer mm to st\n");
		}
//		alt_dcache_flush_all(); //added by aaron
		while(dma_complete == 0);
//		alt_dcache_flush_all();


		alt_u32* current_buffer; //size = 4*width*height;
		buffer_was_full = 0;
		while (HW_video_display_buffer_is_available(display) != 0) {
			buffer_was_full = 1;
			//start_playback_flag = 1;
			//printf("buffer full\n");
		}
		if(buffer_was_full == 1){
			usleep(5000);
			buffer_was_full = 0;
		}

		//Add code to update buffer here.

		//Register updated buffer
		//HW_video_display_register_written_buffer(display);
		//ybcbr to rgb conversion
		current_buffer = HW_video_display_get_buffer(display);





		//ycbcr_start_t = alt_timestamp();

		if (frame_index!=0){

			if (odd_frame == 1){
				for (int h = (9*hCb_size/10); h < hCb_size; h++)
					for (int w = 0; w < wCb_size; w++) {
						int b = h * wCb_size + w;
						//	ycbcr_block_start_t = alt_timestamp();
						ycbcr_to_rgb(h << 3, w << 3, w_size, Yblock[b], Cbblock[b],
								Crblock[b], last_buffer);
						//	ycbcr_block_end_t = alt_timestamp();
						//rgbblock[index] = Cr[y][x] << 16 | Y[y][x] << 8 | Cb[y][x];//= pixel;
					}
			}

			if (odd_frame == 0){
				for (int h = (9*hCb_size/10); h < hCb_size; h++)
					for (int w = 0; w < wCb_size; w++) {
						int b = h * wCb_size + w;
						//		ycbcr_block_start_t = alt_timestamp();
						ycbcr_to_rgb(h << 3, w << 3, w_size, Yblock2[b], Cbblock2[b],
								Crblock2[b], last_buffer);
						//		ycbcr_block_end_t = alt_timestamp();
						//rgbblock[index] = Cr[y][x] << 16 | Y[y][x] << 8 | Cb[y][x];//= pixel;
					}
			}

		}

		if(odd_frame == 0){
			odd_frame = 1;

		}
		else{
			odd_frame =0;

		}
//		alt_dcache_flush_all(); //added by aaron
		last_buffer = current_buffer;
		//switch frames
		//HW_video_display_switch_frames(display);
		//display_end_t = alt_timestamp();

		//frame read, lossless y, lossless cb, lossless cr, idct frame, idct y color, idct block, y2r frame, y2r block, display
		//printf("%d, %d, %d, %d, %d\n",frame_type, frame_size, Ysize, Cbsize, frame_size - Ysize - Cbsize - 16);

		///printf("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,\n",frame_type, frame_size, (file_end_t - file_start_t), (loss_dec_cb_t - loss_dec_y_t), (loss_dec_cr_t - loss_dec_cb_t), (idct_start_t - loss_dec_cr_t), (idct_end_t - idct_start_t), (idct_y_end_t - idct_start_t), (idct_block_end_t-idct_block_start_t), (display_start_t - ycbcr_start_t), (ycbcr_block_end_t - ycbcr_block_start_t), (display_end_t - display_start_t));
//		printf("%ld, %ld, %ld\n",((idct_dma_end - idct_dma_start) ),((cc_time)),((cpu2_lossless_end - cpu2_lossless_start)));

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
	free(Yblock2);
	free(Cbblock2);
	free(Crblock2);
	free(YDCAC);
	free(CbDCAC);
	free(CrDCAC);
	free(Ybitstream);
	free(trailer);

#ifdef PRINT_TO_CONSOLE
	printf("exit decode");
#endif
}


int idct_hw_frame(pdct_block_t* DCAC, pcolor_block_t* block, int block_count, alt_msgdma_dev *read_pointer, alt_msgdma_dev *write_pointer){


	alt_u32 *in_data_ptr = DCAC;
	alt_u32 block_length = 64*block_count;
	alt_u32 *out_data_ptr = block;


	alt_dcache_flush_all();

	alt_msgdma_standard_descriptor mm_to_st_dma_struct;
	alt_msgdma_standard_descriptor st_to_mm_dma_struct;

	if (0 != alt_msgdma_construct_standard_mm_to_st_descriptor(
			read_pointer, &mm_to_st_dma_struct, in_data_ptr,
			block_length*2,
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor mm to st\n");
	}
	if (0 != alt_msgdma_construct_standard_st_to_mm_descriptor(
			write_pointer, &st_to_mm_dma_struct, out_data_ptr,
			block_length,
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor st to mm\n");
	}

	if (0 != alt_msgdma_standard_descriptor_async_transfer(write_pointer,
			&st_to_mm_dma_struct)) {
		//printf("failed starting transfer st to mm\n");
	}

	if (0 != alt_msgdma_standard_descriptor_sync_transfer(read_pointer,
			&mm_to_st_dma_struct)) {
		//printf("failed starting transfer mm to st\n");
	}

//	alt_dcache_flush_all();


	return 0;
}

int idct_test(alt_msgdma_dev *read_pointer, alt_msgdma_dev *write_pointer){
	alt_u32 block_length = 32*sizeof(alt_u32);
	alt_u32 data[32];
	data[0] = 15;
	alt_u32 *data_block = data;
	alt_u32 destination[32];
	destination[0] = 0;

	alt_dcache_flush_all();

	alt_msgdma_standard_descriptor mm_to_st_dma_struct;
	alt_msgdma_standard_descriptor st_to_mm_dma_struct;

	if (0 != alt_msgdma_construct_standard_mm_to_st_descriptor(
			read_pointer, &mm_to_st_dma_struct, data_block,
			block_length,
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor mm to st\n");
	}
	if (0 != alt_msgdma_construct_standard_st_to_mm_descriptor(
			write_pointer, &st_to_mm_dma_struct, destination,
			block_length,
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor st to mm\n");
	}

	if (0 != alt_msgdma_standard_descriptor_sync_transfer(read_pointer,
			&mm_to_st_dma_struct)) {
		printf("failed starting transfer mm to st\n");
	}
	if (0 != alt_msgdma_standard_descriptor_sync_transfer(write_pointer,
			&st_to_mm_dma_struct)) {
		printf("failed starting transfer st to mm\n");
	}

	alt_dcache_flush_all();

	int i;
	printf("DMA TEST: ");
	for( i = 0; i < 8; i++){
		printf("%d ",(int)destination[i]);
	}
	printf("\n");

	return 0;
}

static void write_dma_isr(void* context) {
	dma_complete = 1;
	//printf("reached IDCT DMA isr \n");

}
