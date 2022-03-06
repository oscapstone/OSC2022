#ifndef MAILBOX_H
#define MAILBOX_H


#include "bcm2357.h"
#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif

#define MAILBOX_BASE        (MMIO_BASE + 0x0000b880)

#define MAILBOX_READ        _virt_addr_abs(MAILBOX_BASE, 0x0)
#define MAILBOX_STAT        _virt_addr_abs(MAILBOX_BASE, 0x18)
#define MAILBOX_WRITE       _virt_addr_abs(MAILBOX_BASE, 0x20)
#define MAILBOX_WSTAT       _virt_addr_abs(MAILBOX_BASE, 0x38)

#define MAILBOX_EMPTY               0x40000000
#define MAILBOX_FULL                0x80000000

#define MAILBOX_REQ_CODE            0x00000000

#define TAG_REQ_CODE                0x00000000
#define MAIL_RES_SUCCESS            0x80000000
#define MAIL_RES_ERR                0x80000001
#define TAG_END                     0x0


#define GET_BOARD_REVISION          0x00010002
#define GET_ARM_MEMORY              0x00010005

/*
  ┌────────────────────────────────────────────────────────────────────────────┐
  │                                                                            │
  │ Buffer conntent                                                            │
  │ u32: buffer size in bytes (includingthe header values, the end tag and     │
  │ padding)                                                                   │
  │ u32: buffer request/reponse code                                           │
  │     - Request codes:                                                       │
  │         - 0x00000000: Process request                                      │
  │     - Response codes:                                                      │
  │         - 0x80000000: Request successful                                   │
  │         - 0x80000001: error parsing request buffer                         │
  │ u8 : sequence of concated tags                                             │
  │ u32: 0x0                                                                   │
  │ u8 : padding to aligned 32 bits                                            │
  │                                                                            │
  │                                                                            │
  │ Tag format                                                                 │
  │ u32: tag identifier                                                        │
  │ u32: value buffer size in bytes                                            │
  │ u32: Request codes or Response Codes                                       │
  │     - Request codes:                                                       │
  │         - b31 clear: request                                               │
  │     - Response codes:                                                      │
  │         - b31 set : response                                               │
  │         - b30-b0: value length in bytes                                    │
  │ u8: value buffer                                                           │
  │ u8: padding to align the tag to 32 bits                                    │
  │                                                                            │
  └────────────────────────────────────────────────────────────────────────────┘
 */
// typedef struct {
//     uint32_t bsize;
//     uint32_t bcode;
//     uint32_t payload[100];
// } mailbox_buf_t;

typedef struct mailbox_tag {
    uint32_t tid;
    uint32_t tsize;
    uint32_t tcode;
    uint8_t payload[32];
} mailbox_tag_t;


// typedef mailbox_buf_t* mailbox_buf_ptr_t;

typedef mailbox_tag_t* mailbox_tag_ptr_t;

uint32_t create_mailbox_request(volatile uint32_t p[],
                                    uint32_t tag_id,
                                    uint32_t num_req_data,
                                    uint32_t num_res_data);

uint32_t mailbox_call(uint8_t ch, volatile uint32_t* mailbox);

void get_board_revision();
void get_arm_memory();

void mailbox_dump(uint32_t* buf);


#endif