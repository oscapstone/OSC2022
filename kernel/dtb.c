#include "dtb.h"

char* DTB_ADDRESS;
void* INITRD_ADDR;
void* INITRD_END;

void dtb_init(unsigned long int x0) {
    DTB_ADDRESS = (char*)x0;
    dtb_parser(dtb_get_initrd_callback);
}

void dtb_parser(dtb_callback_t callback) {
    fdt_header* header = (fdt_header*)DTB_ADDRESS;
    // Check magic
    if (BE_to_uint(&header->magic) != 0xd00dfeed)
        return;

    char *dt_sturct = (char*)header + BE_to_uint(&header->off_dt_struct),
         *dt_strings = (char*)header + BE_to_uint(&header->off_dt_strings);
    char* ptr = dt_sturct;
    unsigned int token_type, len;

    while (1) {
        token_type = BE_to_uint(ptr);
        ptr += 4;

        switch (token_type) {
            case FDT_BEGIN_NODE:
                // FDT_BEGIN_NODE followed by node's name
                callback(token_type, ptr, 0);
                // ptr += strlen(node's name) + 1 (null-terminated)
                ptr += strlen(ptr) + 1;
                // 4 bytes alignment, zero padding
                if ((unsigned long int)ptr % 4)
                    ptr += 4 - (unsigned long int)ptr % 4;
                break;
            case FDT_END_NODE:
                // FDT_END_NODE followed immediately by the next token
                callback(token_type, 0, 0);
                break;
            case FDT_PROP:
                // FDT_PROP followed by struct { unsigned int len; unsigned int nameoff;}
                // get len
                len = BE_to_uint(ptr);
                ptr += 4;
                // get name_offset, name = dt_strings + nameoff
                char* name = (char*)dt_strings + BE_to_uint(ptr);
                ptr += 4;
                // struct followed by property's value
                callback(token_type, name, ptr);
                // followed by zeroed padding bytes (if necessary)
                ptr += len;
                if ((unsigned long int)ptr % 4)
                    ptr += 4 - (unsigned long int)ptr % 4;
                break;
            case FDT_NOP:
                // FDT_NOP followed immediately by the next token
                callback(token_type, 0, 0);
                break;
            case FDT_END:
                // end of structure block
                callback(token_type, 0, 0);
                break;
            default:
                break;
        }
        if (token_type == FDT_END)
            break;
    }
}

void dtb_get_initrd_callback(unsigned int token_type, char* name, char* data) {
    if (token_type == FDT_PROP && !strcmp(name, "linux,initrd-start")) {
        INITRD_ADDR = (void *)(PHYS_TO_VIRT((unsigned long long int)BE_to_uint(data)));
        uart_printf("Initramfs address: 0x%x\r\n", INITRD_ADDR);
    }
    if (token_type == FDT_PROP && !strcmp(name, "linux,initrd-end")) {
        INITRD_END = (void *)(PHYS_TO_VIRT((unsigned long long int)BE_to_uint(data)));
    }
}

void dtb_show_callback(unsigned int token_type, char* name, char* data) {
    static unsigned int dtb_tree_level = 0;
    switch (token_type) {
        case FDT_BEGIN_NODE:
            for (unsigned int i = 0; i < dtb_tree_level; i++)
                uart_printf("  ");
            uart_printf("%s{\r\n", name);
            dtb_tree_level++;
            break;

        case FDT_END_NODE:
            dtb_tree_level--;
            for (unsigned int i = 0; i < dtb_tree_level; i++)
                uart_printf("  ");
            uart_printf("}\r\n");
            break;

        case FDT_PROP:
            for (unsigned int i = 0; i < dtb_tree_level; i++)
                uart_printf("  ");
            uart_printf("%s\r\n", name);
            break;
    }
}
