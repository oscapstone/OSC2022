#include "dtb.h"

uint32_t read_dtb32(uint32_t **addr){
    uint32_t data;
    data = *((*addr)++);
    big2little(&data);
    return data;
}

void print_height(int height)
{
    for (;height > 0; height--) uart_puts("    ");
}

void print_prop_value(char *addr, int len)
{
    // check print number or string
    int print_number = false;
    uint32_t number;

    for (int i = 0; i < len; i++) {
        if (!(addr[i] > 31 && addr[i] < 127 || addr[i] == '\0'))
            print_number = true;
    }

    if (print_number) {
        // print number
        if (len == 4) {
            number = read_dtb32(&addr);
            uart_puts("0x");
            uart_hex(number);
        }
        else {
            uart_puts("value");
        }
    }
    else {
        // print string
        for (;len > 1; len--, addr++) {
            if (*addr == '\0')
                uart_puts(", ");
            else
                uart_send(*addr);
        }
    }
}

void big2little(uint32_t *num)
{
    *num = ((*num>>24)&0x000000ff)  |     // move byte 3 to byte 0
           ((*num<< 8)&0x00ff0000)  |     // move byte 1 to byte 2
           ((*num>> 8)&0x0000ff00)  |     // move byte 2 to byte 1
           ((*num<<24)&0xff000000);       // move byte 0 to byte 3
}

void read_fdt_header(void *header, char *addr)
{
    memcpy(header, addr, sizeof(fdt_header_t));
    
    // change big endian to little endian
    for (int i = 0; i < sizeof(fdt_header_t); i += 4) {
        big2little(header);
        header += 4;
    }
}

void dtb_parse( void (*fun)(char *, char *) )
{
    fdt_header_t header;
    fdt_prop_header_t prop_header;
    uint32_t height = 0;
    uint32_t tag;
    const char *struct_addr;
    const char *STRING_BASE;

    // read device tree header
    read_fdt_header(&header, DTB_BASE);
    DTB_END = DTB_BASE + header.totalsize;

    struct_addr = DTB_BASE + header.off_dt_struct;
    STRING_BASE = DTB_BASE + header.off_dt_strings;

    while (1) {
        tag = read_dtb32(&struct_addr);

        if (tag == FDT_BEGIN_NODE) {
            height++;

            // print name
            if (fun == NULL) {
                print_height(height);
                uart_puts(struct_addr);
                uart_puts("\n");
            }
            struct_addr += strlen(struct_addr) + 1;
            struct_addr = ALIGN((uint32_t) struct_addr, 4);
        }
        else if (tag == FDT_END_NODE) {
            height--;
        }
        else if (tag == FDT_PROP) {
            height++;

            prop_header.len = read_dtb32(&struct_addr);
            prop_header.nameoff = read_dtb32(&struct_addr);

            // print property name
            if (fun == NULL) {
                print_height(height);
                uart_puts(STRING_BASE + prop_header.nameoff);
                uart_puts(": < ");

                // print property value as char list
                print_prop_value(struct_addr, prop_header.len);
                uart_puts(" >\n");
            }
            uint32_t value = read_dtb32(&struct_addr);
            struct_addr -= 4;

            if (fun != NULL)
                fun(STRING_BASE + prop_header.nameoff, value);

            // let struct_addr point to next token
            struct_addr += prop_header.len;
            struct_addr = ALIGN((uint32_t) struct_addr, 4);
            
            height--;
        }
        else if (tag == FDT_NOP) {
            // do nothing
        }
        else if (tag == FDT_END) {
            // read dtb finished
            break;
        }
        else {
            // debug
            uart_puts("parse error\ntag: ");
            uart_hex(tag);
            uart_puts("\nstruct addr: ");
            uart_hex(struct_addr);
            exit();
        }
    }

}