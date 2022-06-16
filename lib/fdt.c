#include "fdt.h"
#include "uart.h"
#include "string.h"
#include "address.h"
#include "utils.h"
#include "mmu.h"

uint64 initramfs = (uint64)INITRAMFS_ADDR;

int initramfs_callback(char* data, char* name, char* prop_name) {
    // get init ramfs location
    if(strcmp(name, "chosen") && strcmp(prop_name, "linux,initrd-start")) {
        uint64 value = *((uint64*)data);
        value = SWAP32(value); // big endian
        initramfs = PHY_TO_VIR(value);
        uart_puts("Get initramfs: ");
        uart_hex(value);
        uart_newline();
        return 1;
    }
    else {
        return 0;
    }
}

void fdt_traverse(uint32* addr, int (*callback)(char* prop, char* name, char* prop_name)) {
    // init fdt header
    struct fdt_header head = {
        SWAP32(addr[0]), SWAP32(addr[1]), SWAP32(addr[2]), SWAP32(addr[3]), SWAP32(addr[4]), 
        SWAP32(addr[5]), SWAP32(addr[6]), SWAP32(addr[7]), SWAP32(addr[8]), SWAP32(addr[9])
    };

    // show fdt header
    uart_puts("magic: "); uart_hex(head.magic); uart_newline();
    uart_puts("totalsize: "); uart_hex(head.totalsize); uart_newline();
    uart_puts("off_dt_struct: "); uart_hex(head.off_dt_struct); uart_newline();
    uart_puts("off_dt_strings: "); uart_hex(head.off_dt_strings); uart_newline();
    uart_puts("off_mem_rsvmap: "); uart_hex(head.off_mem_rsvmap); uart_newline();
    uart_puts("version: "); uart_num(head.version); uart_newline();
    uart_puts("last_comp_version: "); uart_num(head.last_comp_version); uart_newline();
    uart_puts("boot_cpuid_phys: "); uart_hex(head.boot_cpuid_phys); uart_newline();
    uart_puts("size_dt_strings: "); uart_hex(head.size_dt_strings); uart_newline();
    uart_puts("size_dt_struct: "); uart_hex( head.size_dt_struct); uart_newline();
    
    char* name, * prop_name, * cnode;
    fdt_prop* prop;
    char* struct_addr = (char*)addr + head.off_dt_struct;
    uint32* node = (uint32*)struct_addr;
    uint32 value, alice;
    int finish = 0;
    while(finish == 0) {
        value = *(node++);
        cnode = (char*)node; // for fetch string & padding
        // printf("NODE: %08x at %08x\n", SWAP32(value), cnode);
        switch(SWAP32(value)) {
            case FDT_BEGIN_NODE:
                // get node name
                name = cnode;
                cnode += strlen(name) + 1; // include '\0'
                uart_puts("name = "); uart_puts(name); uart_newline();
                break;
            case FDT_END_NODE:
                break;
            case FDT_PROP:
                // get property
                prop = (fdt_prop*)cnode;
                prop->len = SWAP32(prop->len);
                prop->nameoff = SWAP32(prop->nameoff); // name string offset
                prop_name = (char*)addr + head.off_dt_strings + prop->nameoff;
                
                uart_puts("prop name = "); uart_puts(prop_name); uart_newline();
                cnode += sizeof(fdt_prop);
                finish = callback(cnode, name, prop_name);
                cnode += prop->len;
                break;   
            case FDT_NOP:
                break;
            case FDT_END:
                finish = 1;
                break;
            default:
                break;
        }

        // 4 bytes alignment
        alice = (cnode - struct_addr) % 4;
        if(alice != 0) {
            cnode += 4 - alice;
        }

        node = (uint32*)cnode;
    }

}