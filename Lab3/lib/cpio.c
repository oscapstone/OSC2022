#include "peripherals/gpio.h"
#include "mini_uart.h"
#include "utils.h"
#include "shell.h"
#include "cpio.h"
#include "timer.h"
#include "sysreg.h"
#include <stddef.h>


void cpio_list() {
    /*
     cpio archive comprises a header record with basic numeric metadata followed by
     the full pathname of the entry and the file data.
    */
    char *addr = CPIO_ADDR;

    while (compare_string((char *)(addr + sizeof(cpio_header)), "TRAILER!!!") != 0) {   
        cpio_header *header = (cpio_header*)addr;
        
        unsigned long pathname_size = hexToDec(header->namesize);
        unsigned long file_size = hexToDec(header->filesize);
        unsigned long headerPathname_size = sizeof(cpio_header) + pathname_size;
        
        align_4(&headerPathname_size); // The pathname is followed by NUL bytes so that the total size of the fixed header plus pathname is a multiple	of four.
        align_4(&file_size);           // Likewise, the	file data is padded to a multiple of four bytes.
        uart_send_string(addr + sizeof(cpio_header)); // print the fine name
        uart_send_string("\r\n");
        addr += (headerPathname_size + file_size);
    }
}

char *findFile(char *name) {
    char *addr = CPIO_ADDR;
    while (compare_string((char *)(addr + sizeof(cpio_header)), "TRAILER!!!") != 0) {
        if ((compare_string((char *)(addr + sizeof(cpio_header)), name) == 0)) {
            return addr;
        }
        cpio_header *header = (cpio_header *)addr;
        unsigned long pathname_size = hexToDec(header->namesize);
        unsigned long file_size = hexToDec(header->filesize);
        unsigned long headerPathname_size = sizeof(cpio_header) + pathname_size;

        align_4(&headerPathname_size); 
        align_4(&file_size);           
        addr += (headerPathname_size + file_size);
    }
    return 0;
}

void cpio_cat(char *filename) {
    char *target = findFile(filename);
    if (target) {
        cpio_header *header = (cpio_header *)target;
        unsigned long pathname_size = hexToDec(header->namesize);
        unsigned long file_size = hexToDec(header->filesize);
        unsigned long headerPathname_size = sizeof(cpio_header) + pathname_size;

        align_4(&headerPathname_size); 
        align_4(&file_size);           

        char *file_content = target + headerPathname_size;
        for (unsigned int i = 0; i < file_size; i++) {
            uart_send(file_content[i]); // print the file content
        }
        uart_send_string("\r\n");
    }
    else {
        uart_send_string("File not found!\n");
    }
}

void load_program() {
    char *target = findFile("user.img");
    if (target) {
        cpio_header *header = (cpio_header *)target;
        unsigned long pathname_size = hexToDec(header->namesize);
        unsigned long file_size = hexToDec(header->filesize);
        unsigned long headerPathname_size = sizeof(cpio_header) + pathname_size;

        align_4(&headerPathname_size); 
        align_4(&file_size);           
        
        unsigned char *addr = (unsigned char *) 0x400000;
        char *file_content = target + headerPathname_size;
        for (unsigned int i = 0; i < file_size; i++) {
            *addr = file_content[i]; // print the file content
            addr++;
        }

        add_timer(read_sysreg(cntfrq_el0) << 1, show_time_elapsed, NULL);
        core_timer_enable();
        asm volatile("mov x0, 0x340  \n");
        asm volatile("msr spsr_el1, x0   \n");
        asm volatile("msr elr_el1, %0    \n" ::"r"(0x400000));
        asm volatile("msr sp_el0, %0    \n" ::"r"(0x400000 + 10000));
        asm volatile("eret   \n");
    }
    else {
        uart_send_string("File not found!\n");
    }
}