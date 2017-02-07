/*
 ****************************
 * ECE423 Video Controller *
 ****************************
 */

/*
 **********************************************************************************
 * This is a rewrite of alt_video_display.c for use with         *
 * a MODULAR SGDMA Controller ie. mSGDMA                                          *
 **********************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <sys/alt_cache.h>
#include <malloc.h>
#include <priv/alt_file.h>
#include "system.h"
#include "ece423_vid_ctl.h" // New mSGDMA Video Controller
#include "i2c.h"

/******************************************************************
 *                    ece423_video_display_init
 *                    ----------------------
 *
 *  This Inits the display controller. Gets memory for the
 *           Frame Bufs & descriptors , Inits the
 *           descriptors, sets size of the Frame Bufs,
 *           Inits all Frame Bufs to Black & Sets Up & STARTS
 *           the mSGDMA.
 *
 *  Returns: Ptr to  display controller structure, or NULL on failure.
 ******************************************************************/
ece423_video_display* ece423_video_display_init(char* sgdma_name, int width,
		int height, int num_buffers) {

	if (ece423_init_hdmi()) {
		printf("Failed to initiate the HDMI chip!\n");
		return 0;
	}

	ece423_video_display* display;
	unsigned int bytes_per_pixel, bytes_per_frame, descriptors_per_frame, i;

	alt_msgdma_dev* pVid_DMA_CSR_Dev; // Ptr to mSGDMA Cont & Status Device

	// PreCalc Values
	bytes_per_pixel = 4;
	bytes_per_frame = ((width * height) * bytes_per_pixel);

	descriptors_per_frame = 1;

	// DON'T EXCEED MAX Frame Bufs
	if (num_buffers > ECE423_VIDEO_DISPLAY_MAX_BUFFERS) {
		printf("The required number of buffers exceeds the max!\n");
		num_buffers = ECE423_VIDEO_DISPLAY_MAX_BUFFERS;
	} else if (num_buffers < 2){
		printf("The number of buffers must be > 2!\n");
		num_buffers = 2;
	}

	// malloc display struct
	display = (ece423_video_display*) malloc(sizeof(ece423_video_display));
	if (!display) {
		return NULL;
	}

	// Init display struct
	display->width = width;
	display->height = height;
	display->num_frame_buffers = num_buffers;
	display->bytes_per_frame = bytes_per_frame;
	display->bytes_per_pixel = bytes_per_pixel;
	display->buffer_being_displayed = 0;
	display->buffer_being_written = (num_buffers > 1) ? 1 : 0; // Init iPrev_Wr_Buf MUST MATCH
	// See iPrev_Wr_Buf in ece423_video_display_buffer_is_available
	display->descriptors_per_frame = descriptors_per_frame;

	// malloc Frame and descriptor Bufs & SetUp Frame Buf Ptrs & Descriptor Ptrs
	if (ece423_video_display_allocate_buffers(display, bytes_per_frame,
			num_buffers)) {
		return NULL;
	}

	pVid_DMA_CSR_Dev = alt_msgdma_open(sgdma_name); // Pt to Cont & Status Dev
	display->mSGDMA = pVid_DMA_CSR_Dev;
	if (pVid_DMA_CSR_Dev == NULL) {
		printf("ERROR ********* UNABLE to OPEN /dev/msgdma_csr\r\n");
		return NULL;
	}

// Construct mSGDMA descriptors for each Frame Buf
	for (i = 0; i < num_buffers; i++) {
		alt_msgdma_construct_standard_mm_to_st_descriptor(pVid_DMA_CSR_Dev,
				display->buffer_ptrs[i]->desc_base,
				(alt_u32 *) display->buffer_ptrs[i]->buffer, bytes_per_frame,
				DESC_CONTROL);
	}

	// Clear all Frame Bufs to Black
	for (i = 0; i < num_buffers; i++) {
		memset((void*) (display->buffer_ptrs[i]->buffer),
		ECE423_VIDEO_DISPLAY_BLACK_8, display->bytes_per_frame);
	}

	// start the mSGDMA by giving it a Descriptor
	while (alt_msgdma_standard_descriptor_async_transfer(pVid_DMA_CSR_Dev,
			display->buffer_ptrs[display->buffer_being_displayed]->desc_base)
			!= 0) {
	}  // Keep Trying until there is room to Transfer another Frame

	return (display);
}

/******************************************************************
 *                 ece423_video_display_register_written_buffer
 *                 -----------------------------------------
 *
 *  This Registers Buf pointed to by buffer_being_written
 *
 ******************************************************************/
void ece423_video_display_register_written_buffer(ece423_video_display* display) {

	//void ret_code = 0;

	/*
	 * Update buffer_being_written
	 * Note: The new buffer_being_written may NOT Yet be FREE
	 * So Call
	 * ece423_video_display_buffer_is_available
	 * to Check Before Drawing in it
	 */
	display->buffer_being_written = (display->buffer_being_written + 1)
			% display->num_frame_buffers;
}

/******************************************************************
 *              ece423_video_display_buffer_is_available
 *              -------------------------------------
 *
 * This Checks If Frame Buf is free to write to
 *             NOTE:buffer_being_written ALREADY points to it.
 *
 *  Returns:  0 - Free Buf available
 *                If Free Buf & NEW Frame HAS been Reg
 *                THEN
 *                buffer_being_displayed is UpDated
 *
 *           -1 - Free Buf not yet available
 *
 ******************************************************************/

int ece423_video_display_buffer_is_available(ece423_video_display* display) {
	int ret_code = 0;

	if (display->num_frame_buffers > 1) {
		if (display->buffer_being_displayed == display->buffer_being_written) // If Frame Buf free to write to
				{
			ret_code = -1; // Free Buf not yet available
		}
	} // END if(display->num_frame_buffers > 1)
	else  // Else Only one display Buf so HAVE TO Overwrite LIVE Buf
	{
		ret_code = 0;
	}

	return (ret_code);
}

void ece423_video_display_switch_frames(ece423_video_display* display) {
	int iNext_Rd_Buf;

	alt_u32 RD_Desc_Fifo_Level = (IORD_ALTERA_MSGDMA_CSR_DESCRIPTOR_FILL_LEVEL(
			display->mSGDMA->csr_base) & ALTERA_MSGDMA_CSR_READ_FILL_LEVEL_MASK)
			>> ALTERA_MSGDMA_CSR_READ_FILL_LEVEL_OFFSET;

	iNext_Rd_Buf = ((display->buffer_being_displayed + 1)
			% display->num_frame_buffers);

	// If there is only one buffer, display it!
	if (display->num_frame_buffers == 1) {
		// Wait until the last buffer is displayed
		while (RD_Desc_Fifo_Level > 0) {
			RD_Desc_Fifo_Level = (IORD_ALTERA_MSGDMA_CSR_DESCRIPTOR_FILL_LEVEL(
					display->mSGDMA->csr_base)
					& ALTERA_MSGDMA_CSR_READ_FILL_LEVEL_MASK)
					>> ALTERA_MSGDMA_CSR_READ_FILL_LEVEL_OFFSET;
		}

		// Transfer Descriptor for Frame to mSGDMA
		while (alt_msgdma_standard_descriptor_async_transfer(display->mSGDMA,
				display->buffer_ptrs[iNext_Rd_Buf]->desc_base) != 0) {
		}  // Keep Trying until there is room to Transfer another Frame
	}

	// Check if there is a new buffer to display
	else if (iNext_Rd_Buf != display->buffer_being_written) {

		// Wait until the last buffer is displayed
		while (RD_Desc_Fifo_Level > 1) {

			RD_Desc_Fifo_Level = (IORD_ALTERA_MSGDMA_CSR_DESCRIPTOR_FILL_LEVEL(
					display->mSGDMA->csr_base)
					& ALTERA_MSGDMA_CSR_READ_FILL_LEVEL_MASK)
					>> ALTERA_MSGDMA_CSR_READ_FILL_LEVEL_OFFSET;
		}

		// Transfer Descriptor for Frame to mSGDMA
		while (alt_msgdma_standard_descriptor_async_transfer(display->mSGDMA,
				display->buffer_ptrs[iNext_Rd_Buf]->desc_base) != 0) {
		}  // Keep Trying until there is room to Transfer another Frame

		display->buffer_being_displayed = iNext_Rd_Buf;
	}

	//printf("Displayed %d - Written %d\n", display->buffer_being_displayed, display->buffer_being_written);
}
/******************************************************************
 *  Function: ece423_video_display_clear_screen
 *
 *  Purpose: Uses the fast memset routine to clear entire Frame Buf
 *             User can specify black(0x00) or white(0xFF).
 *
 ******************************************************************/
void ece423_video_display_clear_screen(ece423_video_display* display,
		char color) {
	memset(
			(void*) (display->buffer_ptrs[display->buffer_being_written]->buffer),
			color, display->bytes_per_frame);
}

/******************************************************************
 *                     PRIVATE FUNCTIONS                           *
 ******************************************************************/

/******************************************************************
 *          ece423_video_display_get_descriptor_span
 *          -------------------------------------
 * 
 * This Calcs the number of bytes required for descriptor storage
 * 
 * The New mSGDMA only needs 1 descriptor per Frame
 *
 * The OLD SGDMA nedded Multiple descriptors per Frame
 * 
 * display->descriptors_per_frame
 *  MUST be SetUp Before Calling this func
 * 
 * Returns: Size (in bytes) of descriptor memory required.
 ******************************************************************/
alt_u32 ece423_video_display_get_descriptor_span(ece423_video_display *display) {
	return ((display->descriptors_per_frame + 2)
			* sizeof(alt_msgdma_standard_descriptor));
}

/******************************************************************
 *              ece423_video_display_allocate_buffers
 *              ----------------------------------
 *
 *  This Allocates memory for Frame Bufs & descriptors
 *  Returns:  0 - Success
 *           -1 - Error allocating memory
 ******************************************************************/
int ece423_video_display_allocate_buffers(ece423_video_display* display,
		int bytes_per_frame, int num_buffers) {
	int i, ret_code = 0;

	/* Allocate Frame Bufs and descriptor Bufs */

	for (i = 0; i < num_buffers; i++) {
		display->buffer_ptrs[i] = (ece423_video_frame*) malloc(
				sizeof(ece423_video_frame)); // malloc Struct with 2 Ptrs

		if (display->buffer_ptrs[i] == NULL) {
			ret_code = -1;
		}

		display->buffer_ptrs[i]->buffer = (void*) alt_uncached_malloc(
				(bytes_per_frame)); // malloc Frame Buf on Heap
//      display->buffer_ptrs[i]->buffer =
//        (void*) malloc(bytes_per_frame); // malloc Frame Buf on Heap
		if (display->buffer_ptrs[i]->buffer == NULL)
			ret_code = -1;

		display->buffer_ptrs[i]->desc_base =
				(alt_msgdma_standard_descriptor*) memalign(32,
						ece423_video_display_get_descriptor_span(display)); // Desc on Heap

		if (display->buffer_ptrs[i]->desc_base == NULL) {
			ret_code = -1;
		}
	}

	return ret_code;
}

// ************************************************************

// Return a pointer to the buffer being written
alt_u32* ece423_video_display_get_buffer(ece423_video_display* display) {

	return (display->buffer_ptrs[display->buffer_being_written]->buffer);
}

// ************************************************************
int ece423_init_hdmi() {
	bool r = 0;
	int slave_addr = 0x39 << 1;
	int chip_id[4];
	int chip_rev[4];

	// Identify adv7513 chip
	r = I2C_Read(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x00, &chip_rev[0]);
	if (!r)
		return -1;

	r = I2C_Read(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0xf6, &chip_id[0]);
	if (!r)
		return -2;

	r = I2C_Read(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0xf5, &chip_id[1]);
	if (!r)
		return -3;

	// Initiate Color Conversion Matrix
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x18, 0xAA);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x19, 0xF8);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x1A, 0x08);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x1B, 0x00);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x1C, 0x00);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x1D, 0x00);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x1E, 0x1a);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x1F, 0x84);

	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x20, 0x1A);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x21, 0x6A);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x22, 0x08);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x23, 0x00);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x24, 0x1D);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x25, 0x50);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x26, 0x04);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x27, 0x23);

	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x28, 0x1F);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x29, 0xFC);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x2A, 0x08);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x2B, 0x00);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x2C, 0x0D);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x2D, 0xDE);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x2E, 0x19);
	I2C_Write(I2C_SCL_BASE, I2C_SDA_BASE, slave_addr, 0x2F, 0x13);

	return 0;
}

