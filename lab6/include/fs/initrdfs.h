#ifndef _INITRDFS_H_
#define _INITRDFS_H_
#include "types.h"
#include "lib/cpio.h"
#include "lib/print.h"
#include "lib/simple_malloc.h"
#include "mm/slab.h"
#include "lib/list.h"
#include "lib/string.h"
#include "debug/debug.h"
#include "peripherals/mini_uart.h"

extern void initrdfs_init(void*);
extern void initrdfs_ls();
extern void initrdfs_cat();
extern void* fdt_initrdfs_callback(uint32_t, fdt_node*, fdt_property*, int32_t);
extern size_t initrdfs_loadfile(char* , uint8_t*, size_t, size_t);
extern size_t initrdfs_filesize(char*);
#endif
