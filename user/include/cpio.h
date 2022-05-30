#ifndef _CPIO_HEADER_
#define _CPIO_HEADER_
#include "utils.h"
#include "dtb.h"
extern uint32_t* cpio_addr;
void cpio_ls();
void cpio_cat();
void cpio_get_addr(char* filedata,unsigned long* filesize);
//void* initramfs_callback(fdt_prop* prop,char * name,uint32_t len_prop);
void* initramfs_start_callback(fdt_prop* prop,char * name,uint32_t len_prop);
void* initramfs_end_callback(fdt_prop* prop,char * name,uint32_t len_prop);

#endif