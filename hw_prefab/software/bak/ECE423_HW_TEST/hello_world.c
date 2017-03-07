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
#include "hw_sd/hw_sd.h"
#include "hw_vid_ctl/hw_vid_ctl.h"


int main()
{
  printf("Hello from Nios II!\n");

  if (SDLIB_Init(SD_CONT_0_BASE) == 1){
    printf("Passed SDLIB!\n");
  }

  printf("2!\n");

  FAT_HANDLE hFAT;
  hFAT = Fat_Mount();

  FAT_BROWSE_HANDLE pFatBrowseHandle;
  printf("Fat_FileBrowseBegin: %d",Fat_FileBrowseBegin(hFAT, &pFatBrowseHandle));

  FILE_CONTEXT pFileContext
  printf("Fat_FileBrowseNext: %d",Fat_FileBrowseNext(pFatBrowseHandle, &pFileContext));

  printf("Fat_CheckExtension: %d",Fat_CheckExtension(pFatBrowseHandle, &pFileContext));

  return 0;
}
