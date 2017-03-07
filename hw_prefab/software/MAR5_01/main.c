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
//#include "mjpeg423_types.h"
#include "decoder/mjpeg423_decoder.h"
//#include "hw_sd/hw_sd.h"
//#include "hw_vid_ctl/hw_vid_ctl.h"

//Timestamp Library
#include <sys/alt_timestamp.h>
//#include "altera_avalon_timer.h"

////Interrupt related includes
//#include "pio_implementation.h"
#include <sys/alt_irq.h>
#include "altera_avalon_timer_regs.h"
//
typedef alt_u32 uint32_t;
//
////Interrupt related function prototypes
static void pio_isr(void* isr_context);
static void timer_isr(void* isr_context);
static int pio_functionality(FAT_HANDLE *hFAT,
		FAT_BROWSE_HANDLE *pFatBrowseHandle, FILE_CONTEXT *pFileContext);
static void peripheral_init(void);

//volatile int count;
//volatile unsigned short int key_base;
//volatile int temp;

 volatile int show_buffers_flag;
 volatile int push_button_flag;
 volatile int video_play_flag;
 volatile int skip_fw_flag;
 volatile int skip_rv_flag;
 volatile int exit_flag;
double time1;
double time2;
double null_execution;
hw_video_display* display;
int fps = 10;

int main() {
	printf("Hello from Nios II bud.\n");
	int sdf = alt_timestamp_start();
	null_execution = alt_timestamp();

	printf("null_execution : %f \n", null_execution);
	if (SDLIB_Init(SD_CONT_0_BASE) == 0) {
		printf("Failed SDLIB!\n");
	}
	//time1= alt_timestamp();
	//printf("time1: %f\n", time1);
	peripheral_init();

	FAT_HANDLE hFAT;
	hFAT = Fat_Mount();

	FAT_BROWSE_HANDLE pFatBrowseHandle;
	if (Fat_FileBrowseBegin(hFAT, &pFatBrowseHandle) == 0) {
		printf("Failed FileBrowseBegin!\n");
	}

	FILE_CONTEXT pFileContext;
	if (Fat_FileBrowseNext(&pFatBrowseHandle, &pFileContext) == 0) {
		printf("Failed FileBrowseNext!\n");
	}

	char* fileName;
	while (1) {
		fileName = Fat_GetFileName(&pFileContext);
		char target[] = "V1_72.MPG";
		if (strcmp(fileName, target) == 0) {
			printf("found %s\n", target);
			break;
		}
		Fat_FileBrowseNext(&pFatBrowseHandle, &pFileContext);
	}
	int num_buffers = 10;
	display = hw_video_display_init(
			VIDEO_DMA_CSR_NAME, 640, 480, num_buffers);

	//time2= alt_timestamp();
	//printf("time2 %f\n", time2);
	FAT_FILE_HANDLE hFileHandle;
	while (1) {

		if (pio_functionality(&hFAT, &pFatBrowseHandle, &pFileContext) == 0){

			fileName = Fat_GetFileName(&pFileContext);
			hFileHandle = Fat_FileOpen(hFAT, fileName);
		}
		if (video_play_flag == 1){
			mjpeg423_decode(hFileHandle, display, &skip_fw_flag, &skip_rv_flag, &exit_flag);
			while(video_play_flag != 0){
			}
			printf("Done playing file.");
		}
	}

	printf("exit main");

	return 0;
}


//---------------------Helper Functions---------------------------------
static int pio_functionality(FAT_HANDLE *hFAT,
		FAT_BROWSE_HANDLE *pFatBrowseHandle, FILE_CONTEXT *pFileContext) {

	if (push_button_flag == 2) {
		printf("entered browsing\n");
		Fat_FileBrowseNext(pFatBrowseHandle, pFileContext);
		char* fileName;
		fileName = Fat_GetFileName(pFileContext);
		char extension[] = ".MPG";
		while (!Fat_CheckExtension(pFileContext, extension)) {
			if(!Fat_FileBrowseNext(pFatBrowseHandle, pFileContext)){
				Fat_FileBrowseBegin(*hFAT, pFatBrowseHandle);
			}

			fileName = Fat_GetFileName(pFileContext);
		}
		printf("Fat_CheckExtension: %d\n",
				Fat_CheckExtension(pFileContext, extension));
		printf("%s \n\n\n", fileName);

		push_button_flag = 0;
		return 0;
	}
	return -1;
//	if (push_button_flag == 1) {
//		play_flag ^= 1;
//	}
//	if (push_button_flag == 4) {
//		skip_fw_flag = 1;
//	}
//	if (push_button_flag == 8) {
//		skip_rv_flag = 1;
//	}


}

//----------------------Init Functions---------------------------------
static void peripheral_init(void) {
	//Pushbuttons
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEY_BASE, 0xf);
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_BASE, 0x0);
	alt_irq_register( KEY_IRQ, (void*) 0, pio_isr);

	int frame_period = TIMER_1_FREQ / fps;

	//Timer
	IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_1_BASE, frame_period);
	IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_1_BASE, frame_period>>16);


	IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_1_BASE, 2);
	IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_1_BASE, 7);

	alt_irq_register( TIMER_1_IRQ, (void*) 0, timer_isr);

	show_buffers_flag = 0;
	push_button_flag = 0;
	video_play_flag = 0;
	exit_flag = 0;

}

////---------------------------ISRs--------------------------------
static void pio_isr(void* isr_context) {

	push_button_flag = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY_BASE);
	if (push_button_flag == 2) {
			exit_flag = 1;
	}
	if (push_button_flag == 1) {
		//printf("play initial %d\n", start_decode_flag);
		video_play_flag = !video_play_flag;
		printf("\nvideo_play_flag = %d\n", video_play_flag);
	}
	if (push_button_flag == 4) {
		skip_fw_flag = 1;
	}
	if (push_button_flag == 8) {
		skip_rv_flag = 1;
	}
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_BASE, 0x0);
	printf("\n Button %d Pressed. \n", push_button_flag);
}

static void timer_isr(void* isr_context) {

	//switch frames
	if(video_play_flag == 1){
		if(show_buffers_flag != 1){
			if(hw_video_display_buffer_is_available(display) != 0){
				show_buffers_flag = 1;
				if(hw_video_display_switch_frames(display) == -1){
					//printf("Dropped frame. Video was paused.\n");
					video_play_flag = 0;
				}
			}
		}else{
			//printf("playback started\n");
			if(hw_video_display_switch_frames(display) == -1){
				//printf("Dropped frame. Video was paused.\n");
				video_play_flag = 0;
			}
		}
	}

	//printf("\n \n \n \nElapsed Time: %d seconds \n \n \n \n \n", count);
	//start_playback_flag = 1;
	IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_1_BASE, 2);

}
