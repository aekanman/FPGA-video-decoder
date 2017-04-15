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
* altera_avalon_mailbox.h                                                     *
*                                                                             *
* Public interfaces to the Software Mailbox component                         *
*                                                                             *
******************************************************************************/
#include <io.h>

#ifndef __ALTERA_AVALON_MAILBOX_SIMPLE_H__
#define __ALTERA_AVALON_MAILBOX_SIMPLE_H__
#include "priv/alt_dev_llist.h"
#include "sys/alt_dev.h"
#include "os/alt_sem.h"
#include <priv/alt_file.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

// Callback routine type definition
typedef void (*altera_mailbox_rx_cb)(void *message);
typedef void (*altera_mailbox_tx_cb)(void *message, int status);

typedef enum mboxtype { MBOX_TX = 0, MBOX_RX } MboxType;
typedef enum eventtype { ISR = 0, POLL } EventType;
/*
 * Mailbox Device Structure
 */
typedef struct altera_avalon_mailbox_dev
{
    alt_dev                 dev;                  /* Device linke-list entry */
    alt_u32                 base;                 /* Base address of Mailbox */
    alt_u32                 mailbox_irq;          /* Mailbox IRQ */
    alt_u32                 mailbox_intr_ctrl_id; /* Mailbox IRQ ID */
    altera_mailbox_tx_cb    tx_cb;                /* Callback routine pointer */
    altera_mailbox_rx_cb    rx_cb;                /* Callback routine pointer */
    MboxType                mbox_type;            /* Mailbox direction */
    alt_u32*                mbox_msg;
    alt_u8                  lock;                 /* Token to indicate mbox_msg already taken */
    ALT_SEM                 (write_lock)          /* Semaphore used to control access to the
                                                   * write in multi-threaded mode */
} altera_avalon_mailbox_dev;

/*
 * Public APIs
 */
altera_avalon_mailbox_dev* altera_avalon_mailbox_open (const char* name, altera_mailbox_tx_cb tx_callback, altera_mailbox_rx_cb rx_callback);
void altera_avalon_mailbox_close (altera_avalon_mailbox_dev* dev);
int altera_avalon_mailbox_send (altera_avalon_mailbox_dev* dev, void* message, int timeout, EventType event);
int altera_avalon_mailbox_retrieve_poll (altera_avalon_mailbox_dev* dev,alt_u32* msg_ptr, alt_u32 timeout);
alt_u32 altera_avalon_mailbox_status (altera_avalon_mailbox_dev* dev);

/*
*   Macros used by alt_sys_init.c
*
*/
#define ALTERA_AVALON_MAILBOX_SIMPLE_INSTANCE(name, dev)        \
static altera_avalon_mailbox_dev dev =                          \
{                                                               \
    {                                                           \
      ALT_LLIST_ENTRY,                                          \
      name##_NAME,                                              \
      altera_avalon_mailbox_open,                               \
      altera_avalon_mailbox_close,                              \
      NULL,                                                     \
      NULL,                                                     \
      NULL,                                                     \
      NULL,                                                     \
      NULL,                                                     \
    },                                                          \
    {                                                           \
    name##_BASE,                                                \
    }                                                           \
}


/*
 * Externally referenced routines
 */
extern void altera_avalon_mailbox_simple_init(altera_avalon_mailbox_dev* dev,
		                                      int intr_controller_id, int irq);

#define ALTERA_AVALON_MAILBOX_SIMPLE_INIT(name, dev)                         \
{                                                                            \
    if (name##_IRQ == ALT_IRQ_NOT_CONNECTED)                                 \
      altera_avalon_mailbox_simple_init(&dev,                                \
                                        0,                                   \
                                        ALT_IRQ_NOT_CONNECTED);              \
    else                                                                     \
      altera_avalon_mailbox_simple_init(&dev,                                \
                                        name##_IRQ_INTERRUPT_CONTROLLER_ID,  \
                                        name##_IRQ);                         \
}

  
#ifdef __cplusplus
}
#endif

#endif /* __ALTERA_AVALON_MAILBOX_H__ */


