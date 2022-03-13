// ref: FreeBSD Manual Pages cpio
#include "cpio.h"

// calculate how many bytes have to padding
// multiplier always 4 in New ASCII Format
unsigned long  align(unsigned long  size, unsigned long  multiplier){
    if(multiplier <= 0) return 0;
    else return (multiplier - (size % multiplier)) % multiplier;	// last % multiplier is for the case: size = n * multiplier
}

void extract_header(struct cpio_newc_header *cpio_addr, struct cpio_info *size_info){
    size_info->file_size = htoi(cpio_addr->c_filesize, 8);			// New ASCII Format uses 8-byte hexadecimal fields for all numbers
    size_info->file_align = align(size_info->file_size, 4);
    size_info->name_size = htoi(cpio_addr->c_namesize, 8);
    size_info->name_align = align(size_info->name_size + CPIO_HEADER_SIZE, 4);
    size_info->offset = CPIO_HEADER_SIZE + size_info->file_size + size_info->file_align + size_info->name_size + size_info->name_align;
}

void cpio_list(){
	char *cur_addr_byte = CPIO_ADDR;													// addr in bytes
	struct cpio_newc_header *cur_addr_struct = (struct cpio_newc_header *)cur_addr_byte	// addr in struct
	struct cpio_info a;
	
	// go through all cpio archive, get all pathname
	while (1) {
		char *pathname = (char *)cur_addr_struct + CPIO_HEADER_SIZE;
		//char *pathname = (char *)((char *)cur_addr_struct + CPIO_HEADER_SIZE);
		if (!strcmp("TRAILER!!!"), pathname)
			break;		// end of the archive
		uart_printf("%s", pathname);
		
		// get total length of the fileaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
		extract_header(cur_addr_struct, &aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa);
		// point to next file
		cur_addr_byte += size_info.offset;
		cur_addr_struct = (struct cpio_newc_header *)cur_addr_byte;
	}

/*
    char *now_ptr = CPIO_ADDR;
    //void *now_ptr = CPIO_ADDR;
    struct cpio_newc_header *cpio_addr = (struct cpio_newc_header* )now_ptr;
    struct cpio_info size_info;
    while(1){
        extract_header(cpio_addr, &size_info);
        char *pathname = (char*)((char*)cpio_addr + CPIO_HEADER_SIZE);
        if(strcmp("TRAILER!!!", pathname) == 0) break;
        uart_puts(pathname);
        uart_puts("\r\n");
        now_ptr += size_info.offset;//next_addr_offset;
        cpio_addr = (struct cpio_newc_header* )now_ptr;
    }
    */
}

void cpio_cat(char *args){
    char *now_ptr = CPIO_ADDR;
    struct cpio_newc_header *cpio_addr = (struct cpio_newc_header* )now_ptr;
    struct cpio_info size_info;
    int flag = 0;
    while(1){
        extract_header(cpio_addr, &size_info);
        char *pathname = (char*)((char*)cpio_addr + CPIO_HEADER_SIZE);
        if(strcmp("TRAILER!!!", pathname) == 0) break;
        if(strcmp(args, pathname) == 0){
            uart_puts_bySize((char*)((char*)cpio_addr + CPIO_HEADER_SIZE + size_info.name_size + size_info.name_align), size_info.file_size);
            uart_puts("\r\n");
            flag = 1;
        }
        now_ptr += size_info.offset;
        cpio_addr = (struct cpio_newc_header* )now_ptr;
    }
    if(flag == 0){
        uart_puts("No such file: ");
        uart_puts(args);
        uart_puts("\r\n");
    }
}
