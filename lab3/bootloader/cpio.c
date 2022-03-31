#include "cpio.h"
#include "string.h"
#include "uart.h"
#include "time.h"

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

void load_app(char *filename){
    char *app_addr = find_app_addr(filename);
    if(app_addr==0){
        uart_puts("app is not found\n");
        return;
    }
    char *target_addr = NEW_ADDR;
    int app_size =  find_app_size(filename);
    while(app_size--){
        *(target_addr++)=*(app_addr++);
    }
    uart_puts("app loading\n");
	unsigned long target = (unsigned long)NEW_ADDR;
	unsigned long target_sp = (unsigned long)NEW_SP;
    core_timer_enable();
    

	asm volatile("mov x0, 0x340			\n");//enable interrupt
	asm volatile("msr spsr_el1, x0		\n");
	asm volatile("msr elr_el1, %0		\n"::"r"(target));
	asm volatile("msr sp_el0, %0		\n"::"r"(target_sp));

    asm volatile("eret					\n");
}

char *find_app_addr(char *filename){
    cpio_newc_header *header_addr = (cpio_newc_header *)cpio_addr;
    char *cpio_ptr = cpio_addr;
    while(1){
        char *pathname = (char*)((char*)header_addr + cpio_size);
        if(!strcmp(pathname, "TRAILER!!!")) break;
        if(!strcmp(pathname, filename)){
            char *file = (char*)((char*)header_addr + extract_header(header_addr, "headernamesize_align"));
            return file;
        }
        cpio_ptr = cpio_ptr + extract_header(header_addr, "headernamesize_align") + extract_header(header_addr, "filesize_align");
        header_addr = (cpio_newc_header *)cpio_ptr;
    }
    return 0;
}

int find_app_size(char *filename){
    cpio_newc_header *header_addr = (cpio_newc_header *)cpio_addr;
    char *cpio_ptr = cpio_addr;
    while(1){
        char *pathname = (char*)((char*)header_addr + cpio_size);
        if(!strcmp(pathname, "TRAILER!!!")) break;
        if(!strcmp(pathname, filename)){
            return extract_header(header_addr, "filesize");
        }
        cpio_ptr = cpio_ptr + extract_header(header_addr, "headernamesize_align") + extract_header(header_addr, "filesize_align");
        header_addr = (cpio_newc_header *)cpio_ptr;
    }
    return 0;
}