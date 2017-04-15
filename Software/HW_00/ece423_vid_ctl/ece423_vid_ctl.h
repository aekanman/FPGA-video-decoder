/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2007 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
*                                                                             *
******************************************************************************/


/*
****************************
* HW Video Controller *
****************************
*/

#ifndef __HW_VID_CTL_H__
#define __HW_VID_CTL_H__

#include <stdio.h>
#include "system.h"
#include "altera_msgdma.h"
#include "altera_msgdma_descriptor_regs.h"
#include "altera_msgdma_csr_regs.h"

/* Maximum number of display buffers the driver will accept */
#define HW_VIDEO_DISPLAY_MAX_BUFFERS 25
#define HW_VIDEO_DISPLAY_BLACK_8 0x00

#define DESC_CONTROL      (ALTERA_MSGDMA_DESCRIPTOR_CONTROL_PARK_READS_MASK | ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_SOP_MASK | ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MASK | ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GO_MASK)  // Also set the park bit so that we can let the mSGDMA worry about the frame DUPLICATION

typedef struct {
  alt_msgdma_standard_descriptor *desc_base; /* Pointer to mSGDMA descriptor chain Changed 4Jun16 */
  void *buffer;                    /* Pointer to video data buffer */
} HW_video_frame;

typedef struct {
  alt_msgdma_dev *mSGDMA;   // Changed 4Jun16
  HW_video_frame* buffer_ptrs[HW_VIDEO_DISPLAY_MAX_BUFFERS];
  int buffer_being_displayed;
  int buffer_being_written;
  int width;
  int height;
  int bytes_per_pixel;
  int bytes_per_frame;
  int num_frame_buffers;
  int descriptors_per_frame;
} HW_video_display;

// -------------
/* Public API */
// -------------
HW_video_display* HW_video_display_init( char* sgdma_name,
                                           int width,
                                           int height,
                                           int num_buffers);
void HW_video_display_register_written_buffer( HW_video_display* display );
int HW_video_display_buffer_is_available( HW_video_display* display );
int HW_video_display_switch_frames(HW_video_display* display);
alt_u32* HW_video_display_Get_CSR_BASE( HW_video_display* display );
alt_u32* HW_video_display_get_buffer( HW_video_display* display);
void HW_video_display_clear_screen ( HW_video_display* frame_buffer,
                                             char color );

// --------------------
/* Private functions */
// --------------------
int HW_init_hdmi();
alt_u32 HW_video_display_get_descriptor_span(HW_video_display *display);

int HW_video_display_allocate_buffers( HW_video_display* display,
                                       int bytes_per_frame,
                                       int num_buffers );

#endif // __HW_VID_CTL_H__
