#ifndef DTB_H
#define DTB_H
#include "uart.h"
#include "string.h"

extern char* DTB_ADDRESS;
extern void* INITRD_ADDR;
extern void* INITRD_END;

// five token types
#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

typedef struct fdt_header {
    unsigned int magic;
    unsigned int totalsize;
    unsigned int off_dt_struct;
    unsigned int off_dt_strings;
    unsigned int off_mem_rsvmap;
    unsigned int version;
    unsigned int last_comp_version;
    unsigned int boot_cpuid_phys;
    unsigned int size_dt_strings;
    unsigned int size_dt_struct;
} fdt_header;

typedef void (*dtb_callback_t)(unsigned int token_type, char *name, char *data);

void dtb_init();
void dtb_parser(dtb_callback_t callback);
void dtb_get_initrd_callback(unsigned int token_type, char *name, char *data);
void dtb_show_callback(unsigned int token_type, char *name, char *data);

#endif