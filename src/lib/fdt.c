#include "fdt.h"
#include "uart.h"
#include "my_string.h"
#include "cpio.h"

extern unsigned long DTB_BASE;

uint32_t fdt_align(uint32_t n)
{
    return (n + 4 - 1) & (~(4 - 1));
}

uint32_t swap_endianess(uint32_t num)
{
    return (num & 0x000000ff) << 24u |
           (num & 0x0000ff00) << 8u |
           (num & 0x00ff0000) >> 8u |
           (num & 0xff000000) >> 24u;
}

void fdt_traverse(void (*callback)(fdt_prop*, char *, char *))
{
    fdt_header *base = (fdt_header *)DTB_BASE;
    if (FDT_MAGIC != swap_endianess(base->magic))
    {
        uart_printf("Error occured\n");
        return;
    }

    char *node_name;
    char *string_addr = ((char *)base + swap_endianess(base->off_dt_strings));
    uint32_t *struct_addr = (uint32_t *)((char *)base + swap_endianess(base->off_dt_struct));

    while (1)
    {
        uint32_t token = swap_endianess(*(struct_addr++));

        switch(token)
        {
        case FDT_BEGIN_NODE:
        {
            node_name = (char *)struct_addr;
            struct_addr += (fdt_align(strlen(node_name) + 1) >> 2);
        }
            break;
        case FDT_PROP:
        {
            fdt_prop *prop = (fdt_prop *)struct_addr;
            char *prop_name = string_addr + swap_endianess(prop->nameoff);

            callback(prop, node_name, prop_name);
            struct_addr += (fdt_align(swap_endianess(prop->len)) + sizeof(fdt_prop)) >> 2;
        }
            break;
        case FDT_END:
            return;
        }
    }
}

void init_callback(fdt_prop *prop, char *node_name, char *prop_name)
{
    if (strncmp(node_name, "chosen", strlen("chosen")) == 0 && strncmp(prop_name, "linux,initrd-start", strlen("linux,initrd-start")) == 0)
        CPIO_BASE = swap_endianess(*((uint32_t *)(prop+1)));
}