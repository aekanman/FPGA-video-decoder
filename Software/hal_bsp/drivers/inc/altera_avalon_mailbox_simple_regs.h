/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2013 Altera Corporation, San Jose, California, USA.           *
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

#ifndef __ALTERA_AVALON_MAILBOX_SIMPLE_REGS_H__
#define __ALTERA_AVALON_MAILBOX_SIMPLE_REGS_H__

#include <io.h>

#define ALTERA_AVALON_MAILBOX_SIMPLE_CMD_OFST          0x0
#define IOADDR_ALTERA_AVALON_MAILBOX_SIMPLE(base)      \
        __IO_CALC_ADDRESS_NATIVE(base, 0)
#define IOWR_ALTERA_AVALON_MAILBOX_CMD(base, data)     \
        IOWR(base, ALTERA_AVALON_MAILBOX_SIMPLE_CMD_OFST, data)
#define IORD_ALTERA_AVALON_MAILBOX_CMD(base)           \
        IORD(base, ALTERA_AVALON_MAILBOX_SIMPLE_CMD_OFST) 
        
#define ALTERA_AVALON_MAILBOX_SIMPLE_PTR_OFST          0x1
#define IOWR_ALTERA_AVALON_MAILBOX_PTR(base, data)     \
        IOWR(base, ALTERA_AVALON_MAILBOX_SIMPLE_PTR_OFST, data)
#define IORD_ALTERA_AVALON_MAILBOX_PTR(base)           \
        IORD(base, ALTERA_AVALON_MAILBOX_SIMPLE_PTR_OFST) 

#define ALTERA_AVALON_MAILBOX_SIMPLE_STS_OFST          0x2
#define ALTERA_AVALON_MAILBOX_SIMPLE_STS_MSK           0x00000003
#define ALTERA_AVALON_MAILBOX_SIMPLE_STS_PENDING_MSK   0x00000001
#define ALTERA_AVALON_MAILBOX_SIMPLE_STS_FULL_MSK      0x00000002
#define IOWR_ALTERA_AVALON_MAILBOX_STS(base, data)     \
        IOWR(base, ALTERA_AVALON_MAILBOX_SIMPLE_STS_OFST, data)
#define IORD_ALTERA_AVALON_MAILBOX_STS(base)           \
        IORD(base, ALTERA_AVALON_MAILBOX_SIMPLE_STS_OFST) 


#define ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST         0x3
#define ALTERA_AVALON_MAILBOX_SIMPLE_INTR_PEN_MSK      0x00000001
#define ALTERA_AVALON_MAILBOX_SIMPLE_INTR_SPACE_MSK    0x00000002
#define IOWR_ALTERA_AVALON_MAILBOX_INTR(base, data)     \
        IOWR(base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST, data)
#define IORD_ALTERA_AVALON_MAILBOX_INTR(base)           \
        IORD(base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST) 


#endif /* __ALTERA_AVALON_MAILBOX_SIMPLE_REGS_H__ */
