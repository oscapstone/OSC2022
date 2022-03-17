#include "cpio.h"
#include "string.h"
#include "uart.h"

void cpio_ls(){
    cpio_newc_header *header_addr = (cpio_newc_header *)cpio_addr;
    char *cpio_ptr = cpio_addr;
    while(1){
        char *pathname = (char*)((char*)header_addr + cpio_size);
        if(!strcmp(pathname, "TRAILER!!!")) break;
        uart_puts(pathname);
        uart_puts("\n");
        cpio_ptr = cpio_ptr + extract_header(header_addr, "headernamesize_align") + extract_header(header_addr, "filesize_align");
        header_addr = (cpio_newc_header *)cpio_ptr;
    }
}

void cpio_cat(char *filename){
    cpio_newc_header *header_addr = (cpio_newc_header *)cpio_addr;
    char *cpio_ptr = cpio_addr;
    while(1){
        char *pathname = (char*)((char*)header_addr + cpio_size);
        if(!strcmp(pathname, "TRAILER!!!")) break;
        if(!strcmp(pathname, filename)){
            char *file = (char*)((char*)header_addr + extract_header(header_addr, "headernamesize_align"));
            uart_puts(file);
            break;
        }
        cpio_ptr = cpio_ptr + extract_header(header_addr, "headernamesize_align") + extract_header(header_addr, "filesize_align");
        header_addr = (cpio_newc_header *)cpio_ptr;
    }
    
}

int extract_header(cpio_newc_header *cpio_header, char *target){
    if(!strcmp(target,"filesize")){
        return hextoint(cpio_header->c_filesize, 8);
    }else if(!strcmp(target,"filesize_align")){
        return align(4, extract_header(cpio_header, "filesize"));
    }else if(!strcmp(target,"namesize")){
        return hextoint(cpio_header->c_namesize, 8);
    }else if(!strcmp(target,"headernamesize")){
        return extract_header(cpio_header, "namesize") + cpio_size;
    }else if(!strcmp(target,"headernamesize_align")){
        return align(4, extract_header(cpio_header, "headernamesize"));
    }
    return 0;
}

int align(int alignment, int size){
    if(alignment == 0) return 0;
    else return (alignment-(size%alignment))%alignment+size;
}