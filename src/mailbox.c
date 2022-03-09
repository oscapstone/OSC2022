#include "mailbox.h"
#include "string.h"
#include "uart.h"
#include "mem_armv8.h"


// volatile uint32_t  __attribute__((aligned(16))) buf[36];

// void get_board_revision() {

//     mailbox_buf_t mbuf;
//     mailbox_tag_t mtag;


//     mbuf.bsize = 7 * 4; // buffer size in bytes
//     mbuf.bcode = MAILBOX_REQ_CODE; // Request cmd
//     // memset(mbuf.payload, 0, 400);

//     mtag.tid = GET_BOARD_REVISION;
//     mtag.tsize = 4;                 //* the maximum of req and res value buffer's length
//     mtag.tcode = TAG_REQ_CODE;
//     mtag.payload[0] = 0;
//     mtag.payload[1] = 0; // value buffer 0
//     mtag.payload[2] = 0;
//     mtag.payload[3] = 0;
//     uint32_t* end = memcpy(mbuf.payload, &mtag, 16);
//     end = TAG_END;



// }

uint32_t create_mailbox_request(volatile uint32_t p[], uint32_t tag_id, uint32_t num_req_data, uint32_t num_res_data) {

    uint32_t i = 0;
    uint32_t num_fill = (num_res_data > num_req_data) ? num_res_data : num_req_data;
    num_fill = num_fill >> 2;

    p[i++] = 0;
    p[i++] = MAILBOX_REQ_CODE;
    p[i++] = tag_id;
    p[i++] = num_res_data;
    p[i++] = num_req_data;
    for(int j=0;j<num_fill;j++) {
        p[i++] = 0x00000000;
    }
    p[i++] = TAG_END;
    p[0] = i*sizeof(*p);

    return p[0];
}


uint32_t mailbox_call(uint8_t ch, volatile uint32_t* mailbox) {

    uint32_t r = ((uint32_t)((uint64_t)(mailbox)&(~0xf)) | (ch & 0x0f));
    do {
        asm volatile("nop");
    } while( (*MAILBOX_WSTAT) & MAILBOX_FULL );

    *MAILBOX_WRITE = r;

    while(1) {
        do{asm volatile("nop"); } while( (*MAILBOX_STAT) & MAILBOX_EMPTY);

        // mailbox_dump(mailbox);

        if(r == *MAILBOX_READ) {
            return mailbox[1] == MAIL_RES_SUCCESS;
        }
    }
    return 0;

}




void get_board_revision() {

    volatile uint32_t buf[36] __attribute__((aligned(16)));


    buf[0] = 7 * 4;
    buf[1] = MAILBOX_REQ_CODE;
    buf[2] = GET_BOARD_REVISION;
    buf[3] = 4;
    buf[4] = TAG_REQ_CODE;
    buf[5] = 0;
    buf[6] = TAG_END;

    if(mailbox_call(0x8, buf)) {
        uart_write("Board version: ");
        uart_hex(buf[5]);
    } else {
        uart_write("Unable get board version\n");
    }

}

void get_arm_memory() {

    volatile uint32_t buf[36] __attribute__((aligned(16)));

    create_mailbox_request(buf, GET_ARM_MEMORY, 0, 8);

    if(mailbox_call(0x8, buf)) {
        uart_write("ARM Memory:\n");
        uart_write("Base Address: ");
        uart_hex(buf[5]);
        uart_write("Size in bytes: ");
        uart_hex(buf[6]);
    } else {
        uart_write("Unable get arm memory information\n");
    }



}



void mailbox_dump(uint32_t* buf) {

    uint32_t size = buf[0];
    size = size>>2;
    for(int i=0;i<size;i++) {
        uart_hex(buf[i]);
    }
}


