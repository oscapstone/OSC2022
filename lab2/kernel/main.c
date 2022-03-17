#include "uart.h"
#include "string.h"

#define HEADER_SIZE 110
#define CPIO_ADDRESS 0x8000000

struct cpio_newc_header {
    char c_magic[6];
    char c_ino[8];
    char c_mode[8];
    char c_uid[8];
    char c_gid[8];
    char c_nlink[8];
    char c_mtime[8];
    char c_filesize[8];
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];
    char c_check[8];
};

int hex_to_int(char *c) {
    int ans = 0;
    for (int i = 0; i < 8; i++) {
        int temp = 0;
        if (c[i] >= 'A' && c[i] <= 'F')
            temp = c[i] - 'A' + 10;
        else
            temp = c[i] - '0';
        for (int j = 0; j < 7 - i; j++)
            temp *= 16;
        ans += temp;
    }
    return ans;
}

int char_to_int(char *c) {
    int ans = 0;
    while (*c != '\0')
        ans = ans * 10 + *(c++) - '0';

    return ans;
}

void get_uart_input(char *input) {
    char tmp;
    int i = 0;
    while (1) {
        tmp = uart_getc();
        if (tmp == '\n') {
            uart_puts("\n");
            input[i] = '\0';
            break;
        } else
            uart_send(tmp);

        input[i] = tmp;
        i++;
    }
    return;
}

void ls() {
    char *cur = (char *)(CPIO_ADDRESS);
    while (1) {
        struct cpio_newc_header *header = (struct cpio_newc_header *)(cur);
        cur += HEADER_SIZE;
        char *name = (char *)(cur);
        if (strcmp(name, "TRAILER!!!") == 0)
            break;
        uart_puts(name);
        uart_puts("\n");
        int namesize = hex_to_int(header->c_namesize);
        cur += namesize;
        if ((HEADER_SIZE + namesize) % 4 != 0)
            cur += 4 - (HEADER_SIZE + namesize) % 4;
        int filesize = hex_to_int(header->c_filesize);
        cur += filesize;
        if (filesize % 4 != 0)
            cur += 4 - filesize % 4;
    }
}

void cat(char *filename) {
    char *cur = (char *)(CPIO_ADDRESS);
    while (1) {
        struct cpio_newc_header *header = (struct cpio_newc_header *)(cur);
        cur += HEADER_SIZE;
        char *name = (char *)(cur);
        if (strcmp(name, "TRAILER!!!") == 0) {
            uart_puts("File not found!\n");
            break;
        }
        int namesize = hex_to_int(header->c_namesize);
        cur += namesize;
        if ((HEADER_SIZE + namesize) % 4 != 0)
            cur += 4 - (HEADER_SIZE + namesize) % 4;
        int filesize = hex_to_int(header->c_filesize);
        if (strcmp(filename, name) == 0) {
            char tmp = *(cur + filesize);
            *(cur + filesize) = '\0';
            uart_puts(cur);
            uart_puts("\n");
            *(cur + filesize) = tmp;
            break;
        }
        cur += filesize;
        if (filesize % 4 != 0)
            cur += 4 - filesize % 4;
    }
}

int malloc_address = 0x9000000;

void *simple_malloc(int size) {
    int cur = malloc_address;
    malloc_address = malloc_address + size;
    return cur;
}

void main() {
    uart_init();

    while (1) {
        uart_puts("\r# ");

        char command[1024];
        get_uart_input(command);

        if (strcmp(command, "ls") == 0) {
            ls();
        } else if (strcmp(command, "cat") == 0) {
            uart_puts("Filename: ");
            char filename[1024];
            get_uart_input(filename);

            cat(filename);
        } else if (strcmp(command, "malloc") == 0) {
            uart_puts("address = 0x");
            uart_hex(&malloc_address);
            uart_puts("\n");

            uart_puts("size: ");
            char size[1024];
            get_uart_input(size);
            int malloc_size = char_to_int(size);
            void *tmp = simple_malloc(malloc_size);
            uart_puts("malloc_address = 0x");
            uart_hex((unsigned int)tmp);
            uart_puts("\n");
        } else
            uart_puts("Error command!\n");
    }
}
