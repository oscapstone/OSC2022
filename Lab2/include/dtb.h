#ifndef __DTB__
#define __DTB__

#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009
#define FDT_MAGIC 0xd00dfeed

extern unsigned long CPIO_BASE;

typedef struct  {
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
}fdt_header;

void fdt_traverse(fdt_header *header, void (*callback)(char*, char*));
void initramfs_callback();
char* dtb_align(char* p);

unsigned int swap_endian(void *p);


#endif
