//
//  mjpeg423_decoder.h
//  mjpeg423app
//
//  Created by Rodolfo Pellizzoni on 12/24/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef mjpeg423app_mjpeg423_decoder_h
#define mjpeg423app_mjpeg423_decoder_h

#include "../common/mjpeg423_types.h"
#include "../hw_sd/hw_sd.h"
#include "../hw_vid_ctl/hw_vid_ctl.h"

void mjpeg423_decode(FAT_FILE_HANDLE file_handle, alt_u32* buffer_out, hw_video_display* display, int* play_flag, int* skip_fw_flag, int* skip_rv_flag, int* exit_flag, volatile int* timer_flag);
void mjpeg423_decode_new(FAT_FILE_HANDLE* file_handle, alt_u32* buffer_out, hw_video_display* display);
void ycbcr_to_rgb(int h, int w, uint32_t w_size, pcolor_block_t Y, pcolor_block_t Cb, pcolor_block_t Cr, alt_u32* rgbblock);
void idct(dct_block_t DCAC, color_block_t block);
void lossless_decode(int num_blocks, void* bitstream, dct_block_t* DCACq, dct_block_t quant, bool P);

#endif
