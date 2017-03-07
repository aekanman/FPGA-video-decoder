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
//#include "ece423_sd/ece423_sd.h"
//#include "ece423_vid_ctl/ece423_vid_ctl.h"

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
static void pio_functionality(FAT_HANDLE *hFAT,
		FAT_BROWSE_HANDLE *pFatBrowseHandle, FILE_CONTEXT *pFileContext);
static void peripheral_init(void);

//volatile int count;
//volatile unsigned short int key_base;
//volatile int temp;

 volatile int timer_flag;
 volatile int push_button_flag;
 volatile int play_flag;
 volatile int skip_fw_flag;
 volatile int skip_rv_flag;
 volatile int exit_flag;
double time1;
double time2;
double null_execution;
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
	int num_buffers = 2;
	ece423_video_display* display = ece423_video_display_init(
			VIDEO_DMA_CSR_NAME, 640, 480, num_buffers);

	//time2= alt_timestamp();
	//printf("time2 %f\n", time2);
	while (1) {

		pio_functionality(&hFAT, &pFatBrowseHandle, &pFileContext);

		fileName = Fat_GetFileName(&pFileContext);
		FAT_FILE_HANDLE hFileHandle = Fat_FileOpen(hFAT, fileName);
				alt_u32* testbuf;
		if (play_flag == 1){
			mjpeg423_decode(hFileHandle, testbuf, display, &play_flag, &skip_fw_flag, &skip_rv_flag, &exit_flag, &timer_flag);
			play_flag = 0;
			}

	}


//	alt_u32* current_buffer; //size = 4*width*height;
//	alt_u32 test_buffer[640 * 480], test_buffer2[640 * 480];
//	int i;
//	for (i = 0; i < 480 * 640; i++) {
//		test_buffer[i] = 0xFFFFFFFF;
//		test_buffer2[i] = 0x0;
//	}

//	while (1) {
//
//		//Get access to free buffer
//		if (ece423_video_display_buffer_is_available(display) != 0) {
//		}
//
//		//Add code to update buffer here.
//
//		//Register updated buffer
//		//ece423_video_display_register_written_buffer(display);
//
//		current_buffer = ece423_video_display_get_buffer(display);
//
//		memcpy(current_buffer, &test_buffer, 480 * 640 * 4);
//		ece423_video_display_register_written_buffer(display);
//
//		//switch frames
//		ece423_video_display_switch_frames(display);
//
//		//Get access to free buffer
//		if (ece423_video_display_buffer_is_available(display) != 0) {
//		}
//
//		//Add code to update buffer here.
//
//		//Register updated buffer
//		//ece423_video_display_register_written_buffer(display);
//
//		current_buffer = ece423_video_display_get_buffer(display);
//
//		memcpy(current_buffer, &test_buffer2, 480 * 640 * 4);
//		ece423_video_display_register_written_buffer(display);
//
//		//switch frames
//		ece423_video_display_switch_frames(display);
//
//		printf("3!\n");
//	}

//  int num_buffers = 2;
//  ece423_video_display* display = ece423_video_display_init(VIDEO_DMA_CSR_NAME, 640, 480, num_buffers);
//  mjpeg423_decode(hFileHandle, testbuf, display);

	/*
	 const int nBufferSize = 1000;
	 char* pBuffer = malloc( sizeof(char) * nBufferSize );

	 if (Fat_FileRead(hFileHandle, pBuffer, nBufferSize) != 0){
	 printf("Opened the file %s\n", fileName);
	 }

	 Fat_FileClose( hFileHandle);
	 printf("Closed the file %s\n", fileName);
	 */

	//char extension[] = "mpg";
	//printf("Fat_CheckExtension: %d\n",Fat_CheckExtension(&pFileContext, &extension));
	//printf("Fat_FileBrowseNext: %d\n", Fat_FileBrowseNext(&pFatBrowseHandle, &pFileContext));
	//char* fileName = Fat_GetFileName(&pFileContext);
	//printf("%s", fileName);
	/*
	 printf("\n");
	 int num_buffers = 2;
	 ece423_video_display* display = ece423_video_display_init(VIDEO_DMA_CSR_NAME, 640, 480, num_buffers);

	 alt_u32* current_buffer; //size = 4*width*height;
	 alt_u32 test_buffer[640*480];
	 int i;
	 for (i = 0; i < 480 * 320; i++) {
	 test_buffer[i] = 0x55555555;
	 }


	 while (1) {

	 //Get access to free buffer
	 if (ece423_video_display_buffer_is_available(display) == 0) {


	 //Add code to update buffer here.

	 //Register updated buffer
	 //ece423_video_display_register_written_buffer(display);

	 current_buffer = ece423_video_display_get_buffer(display);

	 memcpy(current_buffer, &test_buffer, 480*640*4);
	 ece423_video_display_register_written_buffer(display);

	 //switch frames
	 ece423_video_display_switch_frames(display);
	 }



	 printf("3!\n");
	 }
	 */


	printf("exit main");

	return 0;
}


//---------------------Helper Functions---------------------------------
static void pio_functionality(FAT_HANDLE *hFAT,
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
	}
//	if (push_button_flag == 1) {
//		play_flag ^= 1;
//	}
//	if (push_button_flag == 4) {
//		skip_fw_flag = 1;
//	}
//	if (push_button_flag == 8) {
//		skip_rv_flag = 1;
//	}

	push_button_flag = 0;
}

//----------------------Init Functions---------------------------------
static void peripheral_init(void) {
	//Pushbuttons
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEY_BASE, 0xf);
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_BASE, 0x0);
	alt_irq_register( KEY_IRQ, (void*) 0, pio_isr);


	//Timer
	IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_1_BASE, TIMER_1_FREQ);
	IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_1_BASE, TIMER_1_FREQ>>16);


	IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_1_BASE, 2);
	IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_1_BASE, 7);

	alt_irq_register( TIMER_1_IRQ, (void*) 0, timer_isr);

	timer_flag = 0;
	push_button_flag = 0;
	play_flag = 0;
	exit_flag = 0;

}

////---------------------------ISRs--------------------------------
static void pio_isr(void* isr_context) {

	push_button_flag = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY_BASE);
	if (push_button_flag == 2) {
			exit_flag = 1;
	}
	if (push_button_flag == 1) {
		printf("play initial %d\n", play_flag);
		play_flag = !play_flag;
		printf("play %d\n\n", play_flag);
	}
	if (push_button_flag == 4) {
		skip_fw_flag = 1;
	}
	if (push_button_flag == 8) {
		skip_rv_flag = 1;
	}
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_BASE, 0x0);
	printf("\n \n \n \n Button %d Pressed. \n \n \n \n \n", push_button_flag);
}

static void timer_isr(void* isr_context) {
	//printf("\n \n \n \nElapsed Time: %d seconds \n \n \n \n \n", count);
	timer_flag = 1;
	IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_1_BASE, 2);

}
