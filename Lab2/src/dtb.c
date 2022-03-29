#include "../include/dtb.h"
#include "../include/UART.h"
#include "../include/lib.h"

void fdt_traverse(fdt_header *header, void (*callback)(char*, char*)){
    if (swap_endian(&header->magic)!=FDT_MAGIC) 
        return;

    char *fdt_struct = (char*)header + swap_endian(&header->off_dt_struct);    // point to the "structure block"
    char *fdt_strings = (char*)header + swap_endian(&header->off_dt_strings);     // point to the "string block"
    char* p = fdt_struct;

    while(1){
        unsigned int token = swap_endian(p);
        p += 4;

        if(token == FDT_BEGIN_NODE){
            p += Strlen(p) + 1;
            p = dtb_align(p);
        }
        else if(token == FDT_PROP){
            unsigned int len = swap_endian(p);
            p += 4;

            char *name = (char*)fdt_strings + swap_endian(p);  // nameoff
            p += 4;
            callback(name, p);
            p += len;
            p = dtb_align(p);
        }
        else if(token == FDT_END)
            break;

    }
}

void initramfs_callback(char *name, char* p){
    if(Strncmp(name, "linux,initrd-start", 18)==0){
        CPIO_BASE = swap_endian(p);
        UART_put("[initramfs_callback] CPIO_BASE : ");
        UART_hex(CPIO_BASE);
        UART_put("\r\n");
    }
}

char* dtb_align(char *p){
    unsigned long tmp = (unsigned long)p & 0x03;    // last 2 bits
	p -= tmp;

    if(tmp > 0)
        p += 4;

	return (char*)p;
}

unsigned int swap_endian(void *p){
    unsigned char *bytes = (unsigned char*) p;
    return bytes[3] | bytes[2] << 8 | bytes[1] << 16 | bytes[0] << 24;;  // big <-> little
}