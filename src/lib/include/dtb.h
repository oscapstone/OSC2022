#ifndef __DTB__H__
#define __DTB__H__

#include "string.h"
#include "utils.h"
#include "uart.h"
// #include "cpio.h"

unsigned long CPIO_BASE;

#define FDT_BEGIN_NODE  0x00000001
#define FDT_END_NODE    0x00000002
#define FDT_PROP        0x00000003
#define FDT_NOP         0x00000004
#define FDT_END         0x00000009
#define FDT_MAGIC_BIG   0xD00DFEED

#define SWAP_ENDIANNESS(input)  (((input) >> 24) | (((input) & 0x00FF0000) >> 8) | (((input) & 0x0000FF00) << 8) | ((input) << 24))

typedef struct
{
    /* data */
    unsigned int magic;                    // It shall contain the value 0xd00dfeed for big-endian.
    unsigned int totalsize;                // total size in bytes of the devicetree data structure.
    unsigned int off_dt_struct;            // the offset in bytes of the structure block from the beginning of the header.
    unsigned int off_dt_strings;           // the offset in bytes of the string block from the beginning of the header.
    unsigned int off_mem_rsvmap;           // the offset in bytes of the memory reservation block from the beginning of the header.
    unsigned int version;                  // the version of the devicetree.
    unsigned int last_comp_version;        // the lowest version of the devicetree data structure with which the version used is backwards compatible.
    unsigned int boot_cpuid_phys;          // physical ID of the system’s boot CPU.
    unsigned int size_dt_strings;          // This field shall contain the length in bytes of the strings block section of the devicetree blob.
    unsigned int size_dt_struct;           // This field shall contain the length in bytes of the structure block section of the devicetree blob.
}fdt_header;

typedef struct 
{
    /* data */
    unsigned int len;
    unsigned int nameoff;
}fdt_prop;

void fdt_parser(fdt_header *header, void (*callback)(fdt_prop*, char*, char *));
void initramfs_callback(fdt_prop *prop, char *node_name, char *prop_name);

#endif