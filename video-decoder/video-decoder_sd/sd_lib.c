// --------------------------------------------------------------------
// Copyright (c) 2010 by Terasic Technologies Inc. 
// --------------------------------------------------------------------
//
// Permission:
//
//   Terasic grants permission to use and modify this code for use
//   in synthesis for all Terasic Development Boards and Altera Development 
//   Kits made by Terasic.  Other use of this code, including the selling 
//   ,duplication, or modification of any portion is strictly prohibited.
//
// Disclaimer:
//
//   This VHDL/Verilog or C/C++ source code is intended as a design reference
//   which illustrates how these types of functions can be implemented.
//   It is the user's responsibility to verify their design for
//   consistency and functionality through the use of formal
//   verification methods.  Terasic provides no warranty regarding the use 
//   or functionality of this code.
//
// --------------------------------------------------------------------
//           
//                     Terasic Technologies Inc
//                     356 Fu-Shin E. Rd Sec. 1. JhuBei City,
//                     HsinChu County, Taiwan
//                     302
//
//                     web: http://www.terasic.com/
//                     email: support@terasic.com
//
// --------------------------------------------------------------------
//  Revision:
//     2.0:
//          1. Support SDHC (SDCARD >= 4GB)
//          2. Support Single Block Write API
//          3. Add crc16 check for data
//
// --------------------------------------------------------------------

#include <unistd.h>  // usleep#include <stdio.h>
#include "sd_lib.h"
#include "ece423_sd.h"



#ifdef DEBUG_SDCARD
#define SDCARD_TRACE(x)    {DEBUG(("[SD_LIB]")); DEBUG(x);}
#define SDCARD_DEBUG(x)    {DEBUG(("[SD_LIB]")); DEBUG(x);}
#define SDCARD_ERROR(x)    {DEBUG(("[SD_LIB]ERR:")); DEBUG(x);}
#else
#define SDCARD_DEBUG(x)
#define SDCARD_TRACE(x)
#define SDCARD_ERROR(x)
#endif

#define DEBUG_SDCARD_HEX //DEBUG_HEX_PRINTF

struct mmc * sd_drv;
//-------------------------------------------------------------------------
bool SDLIB_Init(int base_addr) {
	//---------------------------------
	//init ocsdc driver
	if (!sd_drv) {
		sd_drv = ocsdc_mmc_init(base_addr, 50000000);
		if(!sd_drv){
			SDCARD_DEBUG(("ocsdc_mmc_init failed\n\r"));
			return FALSE;
		}
		SDCARD_DEBUG(("ocsdc_mmc_init success\n\r"));
	}

	if (sd_drv->has_init)
		return TRUE;
	int err = mmc_init(sd_drv);
	if (err != 0 || sd_drv->has_init == 0) {
		SDCARD_DEBUG(("SDLIB_Init failure\r\n"));
		return FALSE;
	}

	SDCARD_DEBUG(("SDLIB_Init success\r\n"));

#ifdef DEBUG_SDCARD
	print_mmcinfo(sd_drv);
#endif

	return TRUE;
}

bool SDLIB_ReadBlock512(alt_u32 block_number, alt_u8 *buff) {
	//SDCARD_DEBUG(("%d\n", block_number));
	if (mmc_bread(sd_drv, block_number, 1, buff) == 0) {
		SDCARD_DEBUG(("mmc_bread failed\n\r"));
		return FALSE;
	}

	return TRUE;
}

bool SDLIB_ReadBlocks(alt_u32 block_number, alt_u32 n, alt_u8 *buff) {
	//SDCARD_DEBUG(("%d\n", block_number));
	if (mmc_bread(sd_drv, block_number, n, buff) == 0) {
		SDCARD_DEBUG(("mmc_bread failed\n\r"));
		return FALSE;
	}

	return TRUE;
}

