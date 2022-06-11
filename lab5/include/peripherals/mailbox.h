#ifndef _MAILBOX_H_
#define _MAILBOX_H_

#include "types.h"
#include "lib/string.h"
#include "debug/debug.h"
#include "peripherals/iomapping.h"
#include "kernel/sched/sched.h"

#define MBOX_CHANNEL_POWER          0
#define MBOX_CHANNEL_FRAME_BUF      1
#define MBOX_CHANNEL_VIRTUAL_UART   2
#define MBOX_CHANNEL_VCHIQ          3 
#define MBOX_CHANNEL_LEDS           4
#define MBOX_CHANNEL_BUTTONS        5
#define MBOX_CHANNEL_TOUCH_SCREEN   6
#define MBOX_CHANNEL_UNKOWN         7
#define MBOX_CHANNEL_PROPERTY_TAGS  8

enum MBOX_PROPERTY_STATUS {
	MBOX_STATUS_REQUEST = 0,
	MBOX_STATUS_SUCCESS = 0x80000000,
	MBOX_STATUS_ERROR =   0x80000001,
};

enum MBOX_PROPERTY_TAG{
	MBOX_PROPERTY_END =                           0,
	MBOX_GET_FIRMWARE_REVISION =                  0x00000001,

	MBOX_SET_CURSOR_INFO =                        0x00008010,
	MBOX_SET_CURSOR_STATE =                       0x00008011,

	MBOX_GET_BOARD_MODEL =                        0x00010001,
	MBOX_GET_BOARD_REVISION =                     0x00010002,
	MBOX_GET_BOARD_MAC_ADDRESS =                  0x00010003,
	MBOX_GET_BOARD_SERIAL =                       0x00010004,
	MBOX_GET_ARM_MEMORY =                         0x00010005,
	MBOX_GET_VC_MEMORY =                          0x00010006,
	MBOX_GET_CLOCKS =                             0x00010007,
	MBOX_GET_POWER_STATE =                        0x00020001,
	MBOX_GET_TIMING =                             0x00020002,
	MBOX_SET_POWER_STATE =                        0x00028001,
	MBOX_GET_CLOCK_STATE =                        0x00030001,
	MBOX_GET_CLOCK_RATE =                         0x00030002,
	MBOX_GET_VOLTAGE =                            0x00030003,
	MBOX_GET_MAX_CLOCK_RATE =                     0x00030004,
	MBOX_GET_MAX_VOLTAGE =                        0x00030005,
	MBOX_GET_TEMPERATURE =                        0x00030006,
	MBOX_GET_MIN_CLOCK_RATE =                     0x00030007,
	MBOX_GET_MIN_VOLTAGE =                        0x00030008,
	MBOX_GET_TURBO =                              0x00030009,
	MBOX_GET_MAX_TEMPERATURE =                    0x0003000a,
	MBOX_GET_STC =                                0x0003000b,
	MBOX_ALLOCATE_MEMORY =                        0x0003000c,
	MBOX_LOCK_MEMORY =                            0x0003000d,
	MBOX_UNLOCK_MEMORY =                          0x0003000e,
	MBOX_RELEASE_MEMORY =                         0x0003000f,
	MBOX_EXECUTE_CODE =                           0x00030010,
	MBOX_EXECUTE_QPU =                            0x00030011,
	MBOX_SET_ENABLE_QPU =                         0x00030012,
	MBOX_GET_DISPMANX_RESOURCE_MEM_HANDLE =       0x00030014,
	MBOX_GET_EDID_BLOCK =                         0x00030020,
	MBOX_GET_CUSTOMER_OTP =                       0x00030021,
	MBOX_GET_DOMAIN_STATE =                       0x00030030,
	MBOX_SET_CLOCK_STATE =                        0x00038001,
	MBOX_SET_CLOCK_RATE =                         0x00038002,
	MBOX_SET_VOLTAGE =                            0x00038003,
	MBOX_SET_TURBO =                              0x00038009,
	MBOX_SET_CUSTOMER_OTP =                       0x00038021,
	MBOX_SET_DOMAIN_STATE =                       0x00038030,
	MBOX_GET_GPIO_STATE =                         0x00030041,
	MBOX_SET_GPIO_STATE =                         0x00038041,
	MBOX_SET_SDHOST_CLOCK =                       0x00038042,
	MBOX_GET_GPIO_CONFIG =                        0x00030043,
	MBOX_SET_GPIO_CONFIG =                        0x00038043,
	MBOX_GET_PERIPH_REG =                         0x00030045,
	MBOX_SET_PERIPH_REG =                         0x00038045,


	/* Dispmanx TAGS */
	MBOX_FRAMEBUFFER_ALLOCATE =                   0x00040001,
	MBOX_FRAMEBUFFER_BLANK =                      0x00040002,
	MBOX_FRAMEBUFFER_GET_PHYSICAL_WIDTH_HEIGHT =  0x00040003,
	MBOX_FRAMEBUFFER_GET_VIRTUAL_WIDTH_HEIGHT =   0x00040004,
	MBOX_FRAMEBUFFER_GET_DEPTH =                  0x00040005,
	MBOX_FRAMEBUFFER_GET_PIXEL_ORDER =            0x00040006,
	MBOX_FRAMEBUFFER_GET_ALPHA_MODE =             0x00040007,
	MBOX_FRAMEBUFFER_GET_PITCH =                  0x00040008,
	MBOX_FRAMEBUFFER_GET_VIRTUAL_OFFSET =         0x00040009,
	MBOX_FRAMEBUFFER_GET_OVERSCAN =               0x0004000a,
	MBOX_FRAMEBUFFER_GET_PALETTE =                0x0004000b,
	MBOX_FRAMEBUFFER_GET_TOUCHBUF =               0x0004000f,
	MBOX_FRAMEBUFFER_GET_GPIOVIRTBUF =            0x00040010,
	MBOX_FRAMEBUFFER_RELEASE =                    0x00048001,
	MBOX_FRAMEBUFFER_TEST_PHYSICAL_WIDTH_HEIGHT = 0x00044003,
	MBOX_FRAMEBUFFER_TEST_VIRTUAL_WIDTH_HEIGHT =  0x00044004,
	MBOX_FRAMEBUFFER_TEST_DEPTH =                 0x00044005,
	MBOX_FRAMEBUFFER_TEST_PIXEL_ORDER =           0x00044006,
	MBOX_FRAMEBUFFER_TEST_ALPHA_MODE =            0x00044007,
	MBOX_FRAMEBUFFER_TEST_VIRTUAL_OFFSET =        0x00044009,
	MBOX_FRAMEBUFFER_TEST_OVERSCAN =              0x0004400a,
	MBOX_FRAMEBUFFER_TEST_PALETTE =               0x0004400b,
	MBOX_FRAMEBUFFER_TEST_VSYNC =                 0x0004400e,
	MBOX_FRAMEBUFFER_SET_PHYSICAL_WIDTH_HEIGHT =  0x00048003,
	MBOX_FRAMEBUFFER_SET_VIRTUAL_WIDTH_HEIGHT =   0x00048004,
	MBOX_FRAMEBUFFER_SET_DEPTH =                  0x00048005,
	MBOX_FRAMEBUFFER_SET_PIXEL_ORDER =            0x00048006,
	MBOX_FRAMEBUFFER_SET_ALPHA_MODE =             0x00048007,
	MBOX_FRAMEBUFFER_SET_VIRTUAL_OFFSET =         0x00048009,
	MBOX_FRAMEBUFFER_SET_OVERSCAN =               0x0004800a,
	MBOX_FRAMEBUFFER_SET_PALETTE =                0x0004800b,
	MBOX_FRAMEBUFFER_SET_TOUCHBUF =               0x0004801f,
	MBOX_FRAMEBUFFER_SET_GPIOVIRTBUF =            0x00048020,
	MBOX_FRAMEBUFFER_SET_VSYNC =                  0x0004800e,
	MBOX_FRAMEBUFFER_SET_BACKLIGHT =              0x0004800f,

	MBOX_VCHIQ_INIT =                             0x00048010,

	MBOX_GET_COMMAND_LINE =                       0x00050001,
	MBOX_GET_DMA_CHANNELS =                       0x00060001,
};
/*
 * Mailbox Read/Write Peek  Sender  Status    Config
 *    0    0x00       0x10  0x14    0x18      0x1c
 *    1    0x20       0x30  0x34    0x38      0x3c
 */

#define MBOX_READ_CHANNEL_MASK  15
#define MBOX_WRITE_CHANNEL_MASK 15
#define MBOX_WRITE_ADDR_MASK    (~15)
#define MBOX_READ_ADDR_MASK     (~15)
#define MBOX_STATUS_FULL_MASK   0x80000000
#define MBOX_STATUS_EMPTY_MASK  0x40000000
typedef struct{
    uint32_t read; 
    uint32_t unused[5];
    uint32_t status;
    uint32_t config;
    uint32_t write; 
}__attribute__((packed)) MBox_register;

/*       0       4       8      12      16      20      24      28      32
       +---------------------------------------------------------------+
0x00   |                         Buffer Size                           |
       +---------------------------------------------------------------+
0x04   |                   Request/Response Code                       |
       +---------------------------------------------------------------+
0x08   |                             Tags                              |
...    \\                                                             \\
0xXX   |                             Tags                              |
       +---------------------------------------------------------------+
0xXX+4 |                           End Tag (0)                         |
       +---------------------------------------------------------------+
0xXX+8 |                           Padding                             |
...    \\                                                             \\
0xXX+16|                           Padding                             |
       +---------------------------------------------------------------+
*/
#define MBOX_TAG_REQUEST 0
typedef struct{
    uint32_t id;
    uint32_t value_size;
    uint32_t code;
    uint8_t values[0];
}MBox_tag;

typedef struct{
    uint32_t buf_size;
    uint32_t code;
    uint8_t buf[0];
}MBox_buffer;

extern void MBox_get_board_revision(uint32_t*);
extern void MBox_get_arm_memory(uint32_t*);

#endif
