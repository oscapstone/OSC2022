#ifndef _INITRDFS_H_
#define _INITRDFS_H_
#include "utils.h"
#include "types.h"

extern void initrdfs_init(void*);
extern void initrdfs_ls();
extern void initrdfs_cat();
extern void* fdt_initrdfs_callback(uint32_t, fdt_node*, fdt_property*, int32_t);
extern void initrdfs_loadfile(char* , uint8_t*);
#endif
