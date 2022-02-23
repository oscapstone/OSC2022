#ifndef __UTILS__H__
#define __UTILS__H__
#include "mbox.h"
#define GET_BOARD_REVISION  0x00010002
#define GET_BOARD_SERIAL    0x00010004
#define GET_MEMORY_BASE     0x00010005
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

void get_board_revision(unsigned int* board_revision);
void get_board_serial(unsigned int* msb, unsigned int* lsb);
void get_memory_info(unsigned int* mem_base, unsigned int* mem_size);
#endif