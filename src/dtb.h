#ifndef __DTB_H__
#define __DTB_H__

#include "stddef.h"
#include "stdlib.h"

#define FDT_BEGIN_NODE   0x00000001
#define FDT_END_NODE     0x00000002
#define FDT_PROP         0x00000003
#define FDT_NOP          0x00000004
#define FDT_END          0x00000009
#define FDT_MAGIC        0xd00dfeed
#define ZERO_PADDING     0x00000000


typedef struct fdt_header
{
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

} fdt_header_t;

typedef struct fdt_prop_header
{
    uint32_t len;
    uint32_t nameoff;

} fdt_prop_header_t;

void *DTB_BASE;
void *DTB_END;
void big2little(uint32_t *num);
uint32_t read_dtb32(uint32_t **addr);
void print_height(int height);
void print_prop_value(char *addr, int len);
void read_fdt_header(void *header, char *addr);
void dtb_parse( void (*fun)(char *, char *) );

#endif
