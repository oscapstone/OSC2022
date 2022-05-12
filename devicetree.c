#include "devicetree.h"

#include "uart.h"
#include "utils.h"
#define FDT_BEGIN_NODE  0x00000001
#define FDT_END_NODE    0x00000002
#define FDT_PROP        0x00000003
#define FDT_NOP         0x00000004
#define FDT_END         0x00000009

extern void *_fdt_ptr;

int fdt_indent = 0;

typedef struct {
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


unsigned int endiantoi(void* _endian) {
    char* endian = _endian;
    unsigned int tmp = 0;
    tmp |= endian[0] << 24;
    tmp |= endian[1] << 16;
    tmp |= endian[2] << 8;
    tmp |= endian[3];
    return tmp;
}

unsigned int align(unsigned int bytes, int aligned) {
    return (bytes + aligned - 1) & -aligned;
}

void initramfs_callback(int type, char *name, void *data, unsigned int size) {
    switch(type) {
        case FDT_BEGIN_NODE:
            for (int i=0; i<fdt_indent; i++)
                    uart_send('\t');
            printf(name);
            printf(": {");
            uart_send('\n');
            fdt_indent++;
            break;

        case FDT_END_NODE:
            fdt_indent--;
            for (int i=0; i<fdt_indent; i++)
                    uart_send('\t');
            printf("}\n");
            break;

        case FDT_PROP:
            for (int i=0; i<fdt_indent; i++)
                uart_send('\t');
            printf(name);
            uart_send('\n');
            break;

        case FDT_NOP:
            break;

        case FDT_END:
            break;

        default:
            break;
    }
}

void cpio_callback(int type, char *name, void *data, unsigned int size) {
    if (type == FDT_PROP && !strcmp(name, "linux,initrd-start")) {
        cpio_start = (void *)(unsigned long)endiantoi(data);
    }
    if (type == FDT_PROP && !strcmp(name, "linux,initrd-end")) {
        cpio_end = (void *)(unsigned long)endiantoi(data);
    }
}

int fdt_parser(unsigned long _dt_struct, unsigned long _dt_strings, unsigned int totalsize, void (*callback)(int type, char *name, void *data, unsigned int size)) {
    unsigned int end = _dt_struct + totalsize;

    while(_dt_struct < end) {
        unsigned int type = endiantoi((char*)_dt_struct);
        _dt_struct += 4;

        switch (type) {
        case FDT_BEGIN_NODE:
            callback(FDT_BEGIN_NODE, (char*)_dt_struct, 0, 0);
            _dt_struct += align(strlen((char*)_dt_struct)+1, 4);//////////////
            break;
            
        case FDT_END_NODE:
            callback(FDT_END_NODE, 0, 0, 0);
            break;

        case FDT_NOP:
            callback(FDT_NOP, 0, 0, 0);
            break;

        case FDT_PROP: {

            unsigned int size = endiantoi((char*)_dt_struct);
            _dt_struct += 4;
            unsigned int name = endiantoi((char*)_dt_struct);
            _dt_struct += 4;
            callback(FDT_PROP, (char *)(_dt_strings+name), (void *)_dt_struct, size);
            _dt_struct += align(size, 4);
            break;

        }
        case FDT_END:
            callback(FDT_END, 0, 0, 0);
            return 0;
        
        default:
            return -1;
        }
    }
    return -1;
}




int fdt_traverse(void (*callback)(int type, char *name, void *data, unsigned int size)) {
    unsigned long addr = (unsigned long)_fdt_ptr;
    fdt_header* ftd = (fdt_header*)addr;

    if (endiantoi(&ftd->magic) != 0xd00dfeed)
        return 1;
    unsigned int totalsize = (endiantoi(&ftd->totalsize));
    unsigned long off_dt_struct = addr + (endiantoi(&ftd->off_dt_struct));
    unsigned long off_dt_strings = addr + (endiantoi(&ftd->off_dt_strings));

    fdt_start = (void *)(unsigned long)addr;
    fdt_end = (void *)(unsigned long)(addr + totalsize);
    return fdt_parser(off_dt_struct, off_dt_strings, totalsize, callback);
}