#include "mbox.h"
volatile struct mail_packet mbox;

int mbox_call (unsigned char ch) {

    unsigned int addr_channel;
    /* Clear data buffer */
    for (int i = 0; i < MAIL_BODY_BUF_LEN; i++) mbox.body.buffer[i] = 0;
    /* Wait for mbox not full */
    while (mmio_get(MBOX_STATUS) & MBOX_FULL) asm volatile ("nop");
    /* Write the address of message to the mbox with channel identifier */
    addr_channel = (((unsigned int)(unsigned long)&mbox) & 0xFFFFFFF0) | (ch & 0xF);
    mmio_put(MBOX_WRITE, addr_channel);
    /* Wait for our mbox response */
    do {

        while (mmio_get(MBOX_STATUS) & MBOX_EMPTY) asm volatile("nop");

    } while (mmio_get(MBOX_READ) != addr_channel);

    /* check response vaild */
    return mbox.header.code == MBOX_RESPONSE;
}