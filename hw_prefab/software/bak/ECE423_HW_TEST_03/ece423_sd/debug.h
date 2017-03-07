#ifndef DEBUG_H_
#define DEBUG_H_



int myprintf(char *format, ...);
int myprintf_hexarray(unsigned char *pHex, int len);
int myprintf_dwordarray(unsigned int *pArray, int nElementCount);




//============== debug config ===================
#define DEBUG_ENABLED   // major control (turn off all of debug message)
#define xENABLE_UART_DEBUG

#define DEBUG_I2C
//#define DEBUG_FAT
//#define DEBUG_MMC
//#define DEBUG_SDCARD

// debug macro
#ifdef DEBUG_ENABLED
    #define DEBUG(x)               {myprintf x;}  // standard in/out, specifed in project (it could be uart, jtag, or lcd)
    #define DEBUG_HEX_ARRAY(x)     {myprintf_hexarray x;}
    #define DEBUG_DWORD_ARRAY(x)   {myprintf_dwordarray x;}
#else
    #define DEBUG(x)              
    #define DEBUG_HEX_ARRAY(x)    
    #define DEBUG_DWORD_ARRAY(x)  
#endif

#endif /*DEBUG_H_*/
