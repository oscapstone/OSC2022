#ifndef DTB_PARSE_H
#define DTB_PARSE_H

#define dtb_addr (unsigned long*)0x100000

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
void parse_dtb();
void parse_header();
unsigned int convert_big_to_small_endian(unsigned int BE_num);
#endif