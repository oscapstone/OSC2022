#ifndef __DTB_H__
#define __DTB_H__

// #include "stdint.h"
// #include "string.h"

// five token types
#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

typedef struct fdt_header {
    unsigned long magic;
    unsigned long totalsize;
    unsigned long off_dt_struct;
    unsigned long off_dt_strings;
    unsigned long off_mem_rsvmap;
    unsigned long version;
    unsigned long last_comp_version;
    unsigned long boot_cpuid_phys;
    unsigned long size_dt_strings;
    unsigned long size_dt_struct;
} fdt_header;
typedef void (*dtb_callback_t)(unsigned long token_type, char *name, char *data);

void dtb_init();
void dtb_parser(dtb_callback_t callback);
void cat_dt();
void dtb_get_initrd_callback(unsigned long token_type, char *name, char *data);
void dtb_show_callback(unsigned long token_type, char *name, char *data);

#endif