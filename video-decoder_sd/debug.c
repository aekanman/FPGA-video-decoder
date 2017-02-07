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

#include <stdio.h> 
#include <stdarg.h> 
#include "ece423_sd.h"
#include "debug.h"


void debug_output(char *pMessage){
    
    printf(pMessage);
}

int myprintf(char *format, ...){
    int rc;
    char szText[512];
    
    va_list paramList;
    va_start(paramList, format);
    rc = vsnprintf(szText, 512, format, paramList);
    va_end(paramList);
    
    debug_output(szText);

    return rc;    
}


int myprintf_hexarray(unsigned char *pHex, int len){
    int i;
    unsigned char szText[16];
    for(i=0;i<len;i++){
        sprintf(szText, "[%02X]", *(pHex+i));
        DEBUG((szText));
    }
    return len;
}

int  myprintf_dwordarray(unsigned int *pArray, int nElementCount){
    int i;
    char szText[16];
    for(i=0;i<nElementCount;i++){
        sprintf(szText, "[%08X]", *(pArray+i));
        DEBUG((szText));
    }
    return nElementCount;
}
