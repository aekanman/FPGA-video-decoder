/*
 * Worker CPU 2 for the FPGA MJPEG decoder.
 *
 * This core receives commands from the main decoder over mailbox_simple_3,
 * sends acknowledgements over mailbox_simple_2, decodes the chroma lossless
 * stream, and converts the top slice of the frame to the display buffer.
 */

#include <stdio.h>

typedef void (*altera_mailbox_tx_cb)(void *message, int status);

#include "altera_avalon_mailbox_simple.h"
#include "altera_avalon_mailbox_simple_regs.h"
#include "system.h"
#include "common/mjpeg423_types.h"
#include "common/util.h"
#include "decoder/mjpeg423_decoder.h"

#define MAILBOX_TX_DEVICE "/dev/mailbox_simple_2"
#define MAILBOX_RX_DEVICE "/dev/mailbox_simple_3"
#define MAILBOX_TIMEOUT_POLL 0

#define COMMAND_LOSSLESS_DECODE 1
#define COMMAND_COLOR_CONVERT 2

typedef struct {
    int num_blocks;
    uint8_t *bitstream;
    dct_block_t *DCACq;
    uint32_t frame_type;
} ld_mailbox_t;

typedef struct {
    color_block_t *Yblock;
    color_block_t *Cbblock;
    color_block_t *Crblock;
    alt_u32 *current_buffer;
    uint32_t w_size;
    uint32_t h_size;
} cc_mailbox_t;

static void tx_cb(void *report, int status);
static void rx_cb(void *message);
static int open_mailboxes(altera_avalon_mailbox_dev **sender,
        altera_avalon_mailbox_dev **receiver);
static void handle_lossless_decode(alt_u32 *message);
static void handle_color_convert(alt_u32 *message);
static void convert_row_range(cc_mailbox_t *cc_message, int first_row,
        int last_row);
static void acknowledge(altera_avalon_mailbox_dev *sender, alt_u32 *message);

int main(void)
{
    alt_u32 message[2] = {0x00001111, 0xaa55aa55};
    alt_u32 command = 0;
    altera_avalon_mailbox_dev *mailbox_sender;
    altera_avalon_mailbox_dev *mailbox_receiver;

    if (open_mailboxes(&mailbox_sender, &mailbox_receiver) != 0) {
        return 1;
    }

    while (1) {
        altera_avalon_mailbox_retrieve_poll(mailbox_receiver, message,
                MAILBOX_TIMEOUT_POLL);
        command = message[0];

        if (command == COMMAND_LOSSLESS_DECODE) {
            handle_lossless_decode(message);
            acknowledge(mailbox_sender, message);
        }

        if (command == COMMAND_COLOR_CONVERT) {
            handle_color_convert(message);
            acknowledge(mailbox_sender, message);
        }
    }

    return 0;
}

static int open_mailboxes(altera_avalon_mailbox_dev **sender,
        altera_avalon_mailbox_dev **receiver)
{
    *sender = altera_avalon_mailbox_open(MAILBOX_TX_DEVICE, tx_cb, NULL);
    *receiver = altera_avalon_mailbox_open(MAILBOX_RX_DEVICE, NULL, rx_cb);

    if (!*receiver) {
        printf("FAIL: Unable to open receiver mailbox\n");
        return 1;
    }

    if (!*sender) {
        printf("FAIL: Unable to open sender mailbox\n");
        return 1;
    }

    printf("PASS: Opened worker CPU 2 mailboxes\n");
    return 0;
}

static void handle_lossless_decode(alt_u32 *message)
{
    ld_mailbox_t *ld_message = (ld_mailbox_t *)message[1];

    lossless_decode(ld_message->num_blocks, ld_message->bitstream,
            ld_message->DCACq, Cquant, ld_message->frame_type);
    alt_dcache_flush_all();
}

static void handle_color_convert(alt_u32 *message)
{
    cc_mailbox_t *cc_message = (cc_mailbox_t *)message[1];
    int hCb_size = cc_message->h_size / 8;
    int first_row = 0;
    int last_row = (5 * hCb_size / 10) + 3;

    convert_row_range(cc_message, first_row, last_row);
    alt_dcache_flush_all();
}

static void convert_row_range(cc_mailbox_t *cc_message, int first_row,
        int last_row)
{
    int wCb_size = cc_message->w_size / 8;

    for (int h = first_row; h < last_row; h++) {
        for (int w = 0; w < wCb_size; w++) {
            int block_index = h * wCb_size + w;

            ycbcr_to_rgb(h << 3, w << 3, cc_message->w_size,
                    cc_message->Yblock[block_index],
                    cc_message->Cbblock[block_index],
                    cc_message->Crblock[block_index],
                    cc_message->current_buffer);
        }
    }
}

static void acknowledge(altera_avalon_mailbox_dev *sender, alt_u32 *message)
{
    altera_avalon_mailbox_send(sender, message, MAILBOX_TIMEOUT_POLL, ISR);
}

static void tx_cb(void *report, int status)
{
    if (!status) {
        printf("Transfer done");
    } else {
        printf("error in transfer");
    }
}

static void rx_cb(void *message)
{
    alt_u32 *data = (alt_u32 *)message;

    if (message != NULL) {
        printf("Message received from core 2 %d, %d", data[0], data[1]);
    } else {
        printf("Incomplete receive");
    }
}
