#include <dtb.h>
#include <uart.h>
#include <stdint.h>
#include <kmalloc.h>
#include <string.h>
void *_DTB_ADDRESS = (void *)0xffffffff; // set initial value to put the var into data rather than bss

char *fdt_nextnode(char *fdt_addr, char *maxaddr, int* depth)
{
    fdt_addr = (char *)(((((uint64_t)fdt_addr) >> 2) + 1) << 2);
    int end = 0;
    uint64_t proplen;
    while((uint64_t)fdt_addr < (uint64_t)maxaddr && !end) {
        switch (fdt32_ld((uint32_t*)fdt_addr)){
            case FDT_BEGIN_NODE:
                (*depth)++;
                return fdt_addr;
            case FDT_PROP:
                proplen = fdt32_ld((uint32_t*)(fdt_addr+4));
                fdt_addr += 8 + proplen;
                break;
            case FDT_END:
                end = 1;
                break;
            case FDT_END_NODE:
                (*depth)--;
                break;
            case FDT_NOP:
                break;
        }
        //fdt_addr += 4;
        fdt_addr = (char *)(((((uint64_t)fdt_addr) >> 2) + 1) << 2);
    }
    return 0;
}

static inline char *fdt_getname(char *node)
{
    return node + 4;
}

fdt_prop *fdt_getnextprop(char *fdt_addr, char **nexttok)
{
    char *fdt = (char *)_DTB_ADDRESS;
    if((uint64_t)fdt==0xffffffff) return 0;
    char *fdt_string = fdt + fdt_off_dt_strings(fdt);

    //uart_print("fdt_getnextprop addr: 0x");
    //uart_putshex((uint64_t)fdt_addr);

    fdt_addr = (char *)(((((uint64_t)fdt_addr - 1) >> 2) + 1) << 2);
    int end = 0;
    uint64_t proplen;
    fdt_prop *prop;
    while(!end) {
        switch (fdt32_ld((uint32_t*)fdt_addr)){
            case FDT_PROP:
                //uart_print("addr: 0x");
                //uart_putshex((uint64_t)fdt_addr);
                //uart_print("FDT_PROP");
                prop = (fdt_prop*)kmalloc(sizeof(fdt_prop));
                prop->len = fdt32_ld((uint32_t*)(fdt_addr+4));
                prop->name = fdt_string + fdt32_ld((uint32_t *)(fdt_addr+8));
                prop->value = fdt_addr + 12;
                proplen = fdt32_ld((uint32_t*)(fdt_addr+4));
                *nexttok = fdt_addr + 8 + proplen;
                return prop;
            case FDT_END:
            case FDT_BEGIN_NODE:
            case FDT_END_NODE:
                end = 1;
                break;
            case FDT_NOP:
                break;
        }
        //fdt_addr += 4;
        fdt_addr = (char *)(((((uint64_t)fdt_addr) >> 2) + 1) << 2);
    }
    return 0;
}

int fdt_traverse(int (*it)(char* node, const char *name, int depth, void *data), void *data)
{
    char *fdt = (char *)_DTB_ADDRESS;

    //uart_print("Traverse dtb: 0x");
    //uart_putshex((uint64_t)fdt);

    if((uint64_t)fdt==0xffffffff) return -1;

    char *fdt_struc = fdt + fdt_off_dt_struct(fdt);
    char *fdt_string = fdt + fdt_off_dt_strings(fdt);
    char *maxaddr = fdt_struc + fdt_size_dt_struct(fdt);
    
    //uart_print("fdt_size_dt_struct: 0x");
    //uart_putshex((uint64_t)fdt_size_dt_struct(fdt));

    //uart_print("Max Addr: 0x");
    //uart_putshex((uint64_t)maxaddr);

    int depth = 0;
    int ret = 0;
    int end = 0;
    uint64_t proplen;
    char *node_name;
    char *prop_name;
    for(char *node = fdt_nextnode(fdt_struc-1, maxaddr, &depth); node && !end; node = fdt_nextnode(node, maxaddr, &depth)){
        ret = it(node, fdt_getname(node), depth, data);
    }
    return ret;
}