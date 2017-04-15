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
* altera_avalon_mailbox.c                                                     *
*                                                                             *
* API for manipulating the software mailbox associated with the mailbox       *
* component                                                                   *
*                                                                             *
*****************************************************************************/
#include <stddef.h>
#include "nios2.h"
#include "io.h"
#include "sys/alt_irq.h"
#include "sys/alt_errno.h"
#include "altera_avalon_mailbox_simple.h"
#include "altera_avalon_mailbox_simple_regs.h"

ALT_LLIST_HEAD(alt_mailbox_simple_list);
/*
 * Private APIs
 */

/* altera_avalon_mailbox_identify
 * Check an instance open match
 * with the callback register
 */

static void altera_avalon_mailbox_identify (altera_avalon_mailbox_dev *dev)
{
    /* Random signature to test mailbox ownership */
    alt_u32 magic_num = 0x3A11B045;

    IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_PTR_OFST, magic_num);
    if((IORD(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_PTR_OFST)) == magic_num)
    {
        dev-> mbox_type = MBOX_TX;
        /* Clear message_ptr to default */
        IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_PTR_OFST, 0x0);
    } else
    {
	    dev->mbox_type = MBOX_RX;
    }
}

/*
 *   altera_avalon_mailbox_post
 *   This function post message out through sender mailbox
 */
static alt_32 altera_avalon_mailbox_post (altera_avalon_mailbox_dev *dev,  void *message)
{
    alt_u32 *mbox_msg = (alt_u32*) message ;

    if (mbox_msg != NULL) {
        /* When message space available, post the message out */
        IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_PTR_OFST, mbox_msg[1]);
        IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_CMD_OFST, mbox_msg[0]);
        return 0;
    }
    /* Invalid NULL message received */
    return -EINVAL;
}


/* ISR
 * Mailbox Sender Interrupt routine
 */

#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
static void altera_avalon_mailbox_simple_tx_isr(void *context)
#else
static void altera_avalon_mailbox_simple_tx_isr(void *context, alt_u32 id)
#endif
{
    altera_avalon_mailbox_dev *dev = (altera_avalon_mailbox_dev*) context;
    int status = 0;
    alt_u32 data;
    alt_irq_context cpu_sr;
    alt_u32 *message = dev->mbox_msg;

    /* Mask mailbox interrupt */
    data = IORD(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST) &
               (~ALTERA_AVALON_MAILBOX_SIMPLE_INTR_SPACE_MSK);
    IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST, data);

    if (message != NULL)
    {
        /* Post out message requested */
        IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_PTR_OFST, message[1]);
        IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_CMD_OFST, message[0]);
    /*
     * Other interrupts are explicitly disabled if callbacks are registered
     * because there is no guarantee that they are preemption-safe.
     */
        status = (IORD_ALTERA_AVALON_MAILBOX_STS(dev->base)
        		  & ALTERA_AVALON_MAILBOX_SIMPLE_STS_FULL_MSK) >> 1;
        if (dev->tx_cb)
        {
            cpu_sr = alt_irq_disable_all();
  	        (dev->tx_cb)(message, status);
            alt_irq_enable_all(cpu_sr);
        }
        /* Clear mailbox message to NULL after message being posted */
        dev->mbox_msg = NULL;
        dev->lock = 0;
    }
}

/* ISR
 * Mailbox Receiver Interrupt routine
 */

#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
static void altera_avalon_mailbox_simple_rx_isr(void *context)
#else
static void altera_avalon_mailbox_simple_rx_isr(void *context, alt_u32 id)
#endif
{
    altera_avalon_mailbox_dev *dev = (altera_avalon_mailbox_dev*) context;
    alt_irq_context cpu_sr;
    alt_u32 inbox[2];

    inbox[1] = (IORD(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_PTR_OFST));
    inbox[0] = (IORD(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_CMD_OFST));

    /*
     * Other interrupts are explicitly disabled if callbacks are registered
     * because there is no guarantee that they are preemption-safe.
     */
    if (dev->rx_cb)
    {
        cpu_sr = alt_irq_disable_all();
        (dev->rx_cb)(inbox);
        alt_irq_enable_all(cpu_sr);
    }
}

/*
 * Altera avalon mailbox init
 * Initialize mailbox device and identify sender/receiver mailbox
 */
void altera_avalon_mailbox_simple_init (altera_avalon_mailbox_dev *dev,
		                               int intr_id, int irq)
{
    alt_dev_llist_insert((alt_dev_llist*) dev, &alt_mailbox_simple_list);
    
    dev->mailbox_irq    = irq;
    dev->mailbox_intr_ctrl_id = intr_id;
    dev->rx_cb = NULL;
    dev->tx_cb = NULL;
    dev->mbox_msg = NULL;
    
    ALT_SEM_CREATE (&dev->write_lock, 1);

    altera_avalon_mailbox_identify(dev);
}

/*
 * Public APIs
 */


/*
 * altera_avalon_mailbox_open
 * Return - Pointer to the hardware mailbox
 * Search the list of registered mailboxes for one with the supplied name.
 * The return value will be NULL on failure, and non-NULL otherwise.
 */
altera_avalon_mailbox_dev* altera_avalon_mailbox_open (const char *name,
		altera_mailbox_tx_cb tx_callback, altera_mailbox_rx_cb rx_callback)
{
    altera_avalon_mailbox_dev *dev;
    alt_u32 data;

    /* Find requested device */
    dev = (altera_avalon_mailbox_dev*) alt_find_dev (name, &alt_mailbox_simple_list);
    if (dev == NULL)
    {
        return NULL;
    }

    /* Mask mailbox interrupt before ISR is being registered. */
    data = IORD(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST);
    if (dev->mbox_type == MBOX_TX) {
        IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST, \
            (data & ~(ALTERA_AVALON_MAILBOX_SIMPLE_INTR_SPACE_MSK)));
    }
    if (dev->mbox_type == MBOX_RX) {
        IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST, \
            (data & ~(ALTERA_AVALON_MAILBOX_SIMPLE_INTR_PEN_MSK)));
    }

    /* If IRQ not connected, return device pointer without ISR register,
     * in polling mode.
     */
    if (dev->mailbox_irq == ALT_IRQ_NOT_CONNECTED)
        return dev;

    /* For IRQ connected case */

    if ((tx_callback == NULL) && (rx_callback == NULL))
    {
    /* No callback, polling mode */
        return dev;
    }

    /* Ensure user correctly use the mailbox
     * Return - Null if wrong direction set
     */
    if (((dev->mbox_type == MBOX_TX) && (rx_callback != NULL)) ||
	    ((dev->mbox_type == MBOX_RX) && (tx_callback != NULL)))
  	  /* Invalid callback  */
        return NULL;

    /* IRQ is valid register callback
     * to current mailbox device
     */
    dev->tx_cb  = tx_callback;
    dev->rx_cb  = rx_callback;

    /* Register Mailbox's ISR */
    if (dev->mbox_type == MBOX_TX)
    {
    #ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
        alt_ic_isr_register(dev->mailbox_intr_ctrl_id, dev->mailbox_irq, altera_avalon_mailbox_simple_tx_isr,
                            dev, NULL);
    #else
        alt_irq_register(dev->mailbox_irq, dev, altera_avalon_mailbox_simple_tx_isr);
    #endif
    }
  
    if (dev->mbox_type == MBOX_RX)
    {
    #ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
        alt_ic_isr_register(dev->mailbox_intr_ctrl_id, dev->mailbox_irq, altera_avalon_mailbox_simple_rx_isr,
                            dev, NULL);
    #else
        alt_irq_register(dev->mailbox_irq, dev, altera_avalon_mailbox_simple_rx_isr);
    #endif
        /* Enable Receiver interrupt to listen mode */
        data = IORD(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST) |
  	             (ALTERA_AVALON_MAILBOX_SIMPLE_INTR_PEN_MSK);
        IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST, data);
    }
    return dev;
}

/*
 * altera_avalon_mailbox_close
 * Disable mailbox interrupt and irq
 */
void altera_avalon_mailbox_close (altera_avalon_mailbox_dev *dev)
{
    alt_u32 data;
    if ((dev != NULL) && (dev->mailbox_irq != ALT_IRQ_NOT_CONNECTED))
    {
        /* Mask interrupt */
        if (dev->mbox_type == MBOX_TX)
        {
            data = IORD(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST) &
                       (~ALTERA_AVALON_MAILBOX_SIMPLE_INTR_SPACE_MSK);
            IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST, data);
        }
        if (dev->mbox_type == MBOX_RX)
        {
            data = IORD(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST) &
                       (~ALTERA_AVALON_MAILBOX_SIMPLE_INTR_PEN_MSK);
            IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST, data);
        }
  
        /* De-register mailbox irq) */
        if (dev->mailbox_irq != ALT_IRQ_NOT_CONNECTED)
        {
        #ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
            alt_ic_isr_register(dev->mailbox_intr_ctrl_id, dev->mailbox_irq, NULL,
                              dev, NULL);
        #else
            alt_irq_register(dev->mailbox_irq, dev, NULL);
        #endif
        }
        /* De-registering callback to mailbox */
        dev->tx_cb  = NULL;
        dev->rx_cb  = NULL;
    }
}

/*
 *   altera_avalon_mailbox_msg_status
 *   This function read the current message status of the altera mailbox
 *   Return 0 when mailbox is empty or no pending message
 *   Return 1 when mailbox space is full or there is a message pending
 */

alt_u32 altera_avalon_mailbox_status (altera_avalon_mailbox_dev *dev)
{
    alt_u32 mailbox_sts = 0;

    mailbox_sts = (IORD(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_STS_OFST) & ALTERA_AVALON_MAILBOX_SIMPLE_STS_MSK);

    if (dev->mbox_type == MBOX_TX)
        mailbox_sts = (mailbox_sts & ALTERA_AVALON_MAILBOX_SIMPLE_STS_FULL_MSK) >> 1;

    if (dev->mbox_type == MBOX_RX)
        mailbox_sts = mailbox_sts & ALTERA_AVALON_MAILBOX_SIMPLE_STS_PENDING_MSK;

    return mailbox_sts;
}

/*
 * altera_avalon_mailbox_send
 * Send a message to the mailbox.
 * For polling mode, '0' timeout value for infinite polling
 * otherwise timeout when expired
 */
int altera_avalon_mailbox_send
(altera_avalon_mailbox_dev *dev, void *message, int timeout, EventType event)
{
    int status = 0;
    alt_u32 data;
    alt_u32 mbox_status;
  
    /*
     * Obtain the "write_lock"semaphore to ensures 
     * that writing to the device is thread-safe in multi-thread enviroment
     */
    ALT_SEM_PEND (dev->write_lock, 0);

    if (dev->lock || (IORD_ALTERA_AVALON_MAILBOX_STS(dev->base)
    		          & ALTERA_AVALON_MAILBOX_SIMPLE_STS_FULL_MSK))
    {
    	/* dev is lock or no free space to send */
    	return -1;
    }
    else
    {
        dev->mbox_msg = message;
        dev->lock = 1;
    }
    /*
     * Release the write semaphore so that other 
     * threads can access.
     */
    ALT_SEM_POST (dev->write_lock);



    if ((dev->mailbox_irq == ALT_IRQ_NOT_CONNECTED) || (event==POLL))
    {
        /* Polling mode */
        if (timeout ==0)
        {
            do
            {
                mbox_status = altera_avalon_mailbox_status(dev);
            } while (mbox_status);
        } else
        {
            do
            {
                mbox_status = altera_avalon_mailbox_status(dev);
                timeout--;
            } while (mbox_status && (timeout != 0));
            if (timeout == 0)
            {    /* Timeout occur or fail sending */
                return -ETIME;
            }
        }
        status = altera_avalon_mailbox_post (dev, message);
        /* Clear mailbox message to NULL after message being posted */
        dev->mbox_msg = NULL;
        /* Release lock when message posted */
        dev->lock =0;
        return status;
    } else
    {
        /* Enable Sender interrupt */
        data = IORD(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST) |
                   (ALTERA_AVALON_MAILBOX_SIMPLE_INTR_SPACE_MSK);
        IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST, data);
    }
  return 0;
}

/*
 * altera_avalon_mailbox_retrieve_poll
 * 
 * This is a polling function mailbox not in interrupt mode.
 * If a message is available in the mailbox return it otherwise return NULL
 * This function is blocking
 *
 */
int altera_avalon_mailbox_retrieve_poll (altera_avalon_mailbox_dev *dev, alt_u32 *message, alt_u32 timeout)
{
    alt_u32 status = 0;
    alt_u32 data;

    if (dev != NULL && message != NULL)
    {
        /* Mask receiver mailbox interrupt when in polling mode */
        data = IORD(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST);
        IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST,
            (data & (~ALTERA_AVALON_MAILBOX_SIMPLE_INTR_PEN_MSK)));


        /* If timeout is '0', poll till message availabe in mailbox */
        if (timeout == 0)
        {
            do
            {
                status = altera_avalon_mailbox_status (dev);
            } while (status == 0);
        } else
        {
            do
            {
                 status = altera_avalon_mailbox_status (dev);
                 timeout-- ;
            } while ((status == 0) && timeout);
        }

        /* if timeout, status remain 0 */
        if (status)
        {
            message[1] = (IORD(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_PTR_OFST));
            message[0] = (IORD(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_CMD_OFST));
        }
        /* Restore original state of interrupt mask */
        IOWR(dev->base, ALTERA_AVALON_MAILBOX_SIMPLE_INTR_OFST, data);

        /* Return success on complete retrieve message
         * otherwise timeout and exit with error
         */
        if (status)
          return 0;
      }
      /* Invalid Null dev and message */
      message[1] = 0;
      message[0] = 0;
      return -EINVAL;
}
