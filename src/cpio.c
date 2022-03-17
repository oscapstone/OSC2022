#include "cpio.h"

void cpio_init(char* name, char *base_addr)
{
    if (strcmp(name, "linux,initrd-start") == 0) {
        CPIO_BASE = base_addr;
    }
}

void cpio_cat(char *filename)
{
    char *fs = (char *) CPIO_BASE;
    cpio_fp_t fp;

    while (1) {
        cpio_read_file(&fs, &fp);

        if (strncmp(fp.filename, filename, ascii2int(fp.header->c_namesize, 8)) == 0) {
            uart_puts(fp.data);
            return;
        }
        if (strncmp(fp.filename, "TRAILER!!!", 10) == 0) break;
    }
    uart_puts("file not found!\n");
}

void cpio_ls()
{
    char *fs = (char *) CPIO_BASE;
    cpio_fp_t fp;

    while (1) {
        cpio_read_file(&fs, &fp);

        if (strncmp(fp.filename, "TRAILER!!!", 10) == 0) break;

        uart_puts(fp.filename);
        uart_puts("\n");
    }
}

void cpio_read_file(void **addr, cpio_fp_t *fp)
{
    char *fs = (char *) *addr;

    // read file header
    fp->header = (cpio_newc_header_t *) fs;
    fs += 110;

    // read file name
    fp->filename = fs;
    fs += ascii2int(fp->header->c_namesize, 8);
    fs = ALIGN((unsigned long)fs, 4);

    // read file data
    fp->data = fs;
    
    // let addr point to next header
    fs += ascii2int(fp->header->c_filesize, 8);
    fs = ALIGN((unsigned long)fs, 4);
    *addr = fs;
}

int ascii2int(char *str, int len)
{
    int integer = 0;

    for (int i = 0; i < len; i++) {
        integer *= 16;
        integer += (str[i] > '9')? str[i] - 'A' + 10: str[i] - '0';
    }

    return integer;
}

