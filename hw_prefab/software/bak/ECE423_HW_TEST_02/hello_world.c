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
#include "mjpeg423_types.h"
#include "mjpeg423_decoder.h"
#include "hw_sd/hw_sd.h"
#include "hw_vid_ctl/hw_vid_ctl.h"

typedef alt_u32 uint32_t;

int main()
{
  printf("Hello from Nios II!\n");

  if (SDLIB_Init(SD_CONT_0_BASE) == 0){
    printf("Failed SDLIB!\n");
  }

  //printf("2!\n");

  FAT_HANDLE hFAT;
  hFAT = Fat_Mount();

  FAT_BROWSE_HANDLE pFatBrowseHandle;
  if (Fat_FileBrowseBegin(hFAT, &pFatBrowseHandle) == 0){
    printf("Failed FileBrowseBegin!\n");
  }

  FILE_CONTEXT pFileContext;
  if (Fat_FileBrowseNext(&pFatBrowseHandle, &pFileContext) == 0){
    printf("Failed FileBrowseNext!\n");
  }

  char* fileName;
  while(1){
	  fileName = Fat_GetFileName(&pFileContext);
	  char target[] = "V1_72.MPG";
	  if(strcmp(fileName, target) == 0){
		  printf("found %s\n", target);
		  break;
	  }
	  Fat_FileBrowseNext(&pFatBrowseHandle, &pFileContext);
  }

  FAT_FILE_HANDLE hFileHandle = Fat_FileOpen(hFAT, fileName);
  alt_u32* testbuf;
  mjpeg423_decode(hFileHandle, testbuf);

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
  hw_video_display* display = hw_video_display_init(VIDEO_DMA_CSR_NAME, 640, 480, num_buffers);

	alt_u32* current_buffer; //size = 4*width*height;
	alt_u32 test_buffer[640*480];
	int i;
	for (i = 0; i < 480 * 320; i++) {
		test_buffer[i] = 0x55555555;
	}


	while (1) {

		//Get access to free buffer
		if (hw_video_display_buffer_is_available(display) == 0) {


			//Add code to update buffer here.

			//Register updated buffer
			//hw_video_display_register_written_buffer(display);

			current_buffer = hw_video_display_get_buffer(display);

			memcpy(current_buffer, &test_buffer, 480*640*4);
			hw_video_display_register_written_buffer(display);

			//switch frames
			hw_video_display_switch_frames(display);
		}



		printf("3!\n");
	}
	*/
  printf("exit main");

  return 0;
}
