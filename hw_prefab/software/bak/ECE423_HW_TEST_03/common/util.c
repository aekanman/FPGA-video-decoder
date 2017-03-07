//
//  util.c
//  mjpeg423app
//
//  Created by Rodolfo Pellizzoni on 12/24/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "util.h"

void error_and_exit(const char* str){
    printf("Error: %s\n", str);
    exit(-1);
}

void print_block(pcolor_block_t b)
{
    for (int row = 0; row < 8; row++)
        for(int column = 0; column < 8; column++){
            if(column != 7) printf("%d,", b[row][column]);
            else printf("%u\n", b[row][column]);
        } 
}

void print_dct(pdct_block_t b)
{
    for (int row = 0; row < 8; row++)
        for(int column = 0; column < 8; column++){
            if(column != 7) printf("%d,", b[row][column]);
            else printf("%d\n", b[row][column]);
        } 
}

void print_bitstream(int num_bytes, void* bitstream)
{
    for (int index = 0; index < num_bytes; index++)
        for(int x = 0; x < 8 ; x++)
            printf("%u", (uint8_t)((((uint8_t*)bitstream)[index]) << x) >> 7);
    printf("\n");
}
