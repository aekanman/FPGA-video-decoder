/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
typedef void (*altera_mailbox_tx_cb)(void *message,int status);
#include "altera_avalon_mailbox_simple.h"
#include "altera_avalon_mailbox_simple_regs.h"
#include "system.h"
#include "common/mjpeg423_types.h"
#include "common/util.h"
#include <stdlib.h>
#include <string.h>
int hCb_size;// = cc_message->h_size / 8;           //number of chrominance blocks
int wCb_size;// = cc_message->w_size / 8;
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

ld_mailbox_t* ld_message;
cc_mailbox_t* cc_message;

void tx_cb (void* report, int status) {
 if (!status) {
 printf ("Transfer done");
 } else {
 printf ("error in transfer");
 }
}

void rx_cb (void* message) {
	alt_u32* data = (alt_u32*) message;
	if (message!= NULL) {

	 printf ("Message received from core 1 %d, %d", *(data), *(data+4));
	 } else {
	 printf ("Incomplete receive");
	 }
}
int main()
{

	alt_u32 message[2] = {0x00001111, 0xaa55aa55};
	int timeout = 50000;
	alt_u32 status;
	altera_avalon_mailbox_dev* mailbox_sender;
	altera_avalon_mailbox_dev* mailbox_rcv;


	mailbox_sender = altera_avalon_mailbox_open("/dev/mailbox_simple_1", tx_cb,
	NULL);
	mailbox_rcv = altera_avalon_mailbox_open("/dev/mailbox_simple_0", NULL,
	rx_cb);
	if (!mailbox_rcv){
	 printf ("FAIL: Unable to open mailbox_simple");
	 return 1;
	 }


	 if (!mailbox_sender){
	 printf ("FAIL: Unable to open mailbox_simple\n");
	 return 1;
	 }
	 else{
		 printf ("PASS: Opened mailbox_simple\n");

	 }

	 timeout = 0;
	 alt_u32 command = 0;
	 alt_u32 *pointer_to_data;
	 while (1){
	 	 altera_avalon_mailbox_retrieve_poll (mailbox_rcv,message, timeout);
	    if (message == NULL) {
	        printf ("Receive Error");
	    } else {
	    	//alt_dcache_flush_all(); //added by aaron
	    	command = message[0];
	     // printf ("bitstream %d, DCACq %d, frame_type  %d \n",ld_message->bitstream,ld_message->DCACq,ld_message->frame_type);

	      if(command==1){
		  command = 0;
		  ld_message= message[1];
	      lossless_decode(ld_message->num_blocks, ld_message->bitstream, ld_message->DCACq, Yquant,
	    		  ld_message->frame_type);
	      alt_dcache_flush_all();
	      status = altera_avalon_mailbox_send(mailbox_sender,message,timeout,ISR);
	      }

	      if(command==2){
	      cc_message = message[1];
	       hCb_size = cc_message->h_size / 8;           //number of chrominance blocks
	       wCb_size = cc_message->w_size / 8;
	     //   for (int h = hCb_size; h < 4*hCb_size/5; h++)
		  for (int h = (5*hCb_size/10)+3;h < (8*hCb_size/10); h++)
					for (int w = 0; w < wCb_size; w++) {
						int b = h * wCb_size + w;
						ycbcr_to_rgb(h << 3, w << 3, cc_message->w_size, cc_message->Yblock[b], cc_message->Cbblock[b],
								cc_message->Crblock[b], cc_message->current_buffer);
					}
		  alt_dcache_flush_all(); //added by aaron
		  status = altera_avalon_mailbox_send(mailbox_sender,message,timeout,ISR);
	      }


	    }

	 }



  return 0;
}
