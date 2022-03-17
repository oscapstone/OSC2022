#ifndef FLATTENED_DEVICETREE_H
#define FLATTENED_DEVICETREE_H
#include "stdint.h"

extern uint64_t  bootloader_info[4];
#define FDT_BEGIN_NODE_BIG 0x1000000
#define FDT_END_NODE_BIG 0x2000000
#define FDT_PROP_BIG 0x3000000
#define FDT_NOP_BIG 0x4000000
#define FDT_END_BIG 0x9000000

typedef struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
} FDT_HEADER;

typedef struct fdt_prop_header {
    uint32_t len;
    uint32_t nameoff;
} FDT_PROP_HEADER;

extern FDT_HEADER *fdt_head;

uint32_t uint32_t_b2l (uint32_t num);

uint32_t get_fdt_header_magic ();
uint32_t get_fdt_header_totalsize ();
uint32_t get_fdt_header_off_dt_struct ();
uint32_t get_fdt_header_off_dt_strings ();
uint32_t get_fdt_header_off_mem_rsvmap ();
uint32_t get_fdt_header_version ();
uint32_t get_fdt_header_last_comp_version ();
uint32_t get_fdt_header_boot_cpuid_phys ();
uint32_t get_fdt_header_size_dt_strings ();
uint32_t get_fdt_header_size_dt_struct ();

void show_fdt_info ();
void show_all_fdt ();
#endif
