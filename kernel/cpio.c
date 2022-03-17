#include "cpio.h"

cpio_newc_header* header;

void cpio_init() {
    header = malloc(0x800);
}

void cpio_newc_parser(void* callback, char* param) {
    char *cpio_ptr = INITRD_ADDR,
         *file_name, *file_data;
    uint32_t name_size, data_size;
    cpio_newc_header* header = malloc(CPIO_NEWC_HEADER_SIZE);

    while (1) {
        // cpio_ptr move 0x6e
        cpio_newc_parse_header(&cpio_ptr, &header);
        name_size = hex_ascii_to_uint32(header->c_namesize, sizeof(header->c_namesize));
        data_size = hex_ascii_to_uint32(header->c_filesize, sizeof(header->c_filesize));
        // path align with header
        cpio_newc_parse_data(&cpio_ptr, &file_name, name_size, CPIO_NEWC_HEADER_SIZE);
        // data align itself
        cpio_newc_parse_data(&cpio_ptr, &file_data, data_size, 0);

        // cpio archive must have last path_name = TRAILER!!!
        if (!strcmp(file_name, "TRAILER!!!"))
            break;

        // callback function
        ((void (*)(char* param, cpio_newc_header* header, char* file_name, uint32_t name_sizez, char* file_data, uint32_t data_size))callback)(
            param, header, file_name, name_size, file_data, data_size);
    }
}

void cpio_newc_parse_header(char** cpio_ptr, cpio_newc_header** header) {
    *header = *cpio_ptr;
    *cpio_ptr += sizeof(cpio_newc_header);
}

void cpio_newc_parse_data(char** cpio_ptr, char** buf, uint32_t size, uint32_t offset) {
    *buf = *cpio_ptr;
    while ((size + offset) % 4 != 0) {
        size += 1;
    }
    *cpio_ptr += size;
}

void cpio_ls_callback(char* param, cpio_newc_header* header, char* file_name, uint32_t name_size, char* file_data, uint32_t data_size) {
    uart_putc(file_name, name_size);
    uart_write_string("\r\n");
}

void cpio_cat_callback(char* param, cpio_newc_header* header, char* file_name, uint32_t name_size, char* file_data, uint32_t data_size) {
    if (strcmp(param, file_name))
        return;
    uart_write_string("Filename: ");
    uart_putc(file_name, name_size);
    uart_write_string("\r\n");
    uart_putc(file_data, data_size);
    uart_write_string("\r\n");
}
