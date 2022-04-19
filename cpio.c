#include "cpio.h"

#include "devicetree.h"
#include "uart.h"
#include "utils.h"
#include "shell.h"
#include "exception.h"

//Cpio New ASCII Format https://www.freebsd.org/cgi/man.cgi?query=cpio&sektion=5
typedef struct{
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
} cpio_t;

unsigned long strtoi(char* str) {
	unsigned int tmp=0;
	for(int i=0; i<8; ++i) {
		if(str[i]>='0' && str[i]<='9') {
			tmp = tmp*16 + str[i] - '0';
		}
		else {
			tmp = tmp*16 + str[i] - 'a'+10;
		}
	}
	return tmp;
}

void cat_cpio(cpio_t* addr, char* target){
	while(1){
		unsigned long nsize, fsize;
		nsize=strtoi(addr->c_namesize);
		fsize=strtoi(addr->c_filesize);

		// total size of the fixed header plus pathname is a multiple of four
		if((sizeof(cpio_t) + nsize) & 3)
			nsize += 4 - ((sizeof(cpio_t) + nsize) & 3);
		if(fsize & 3)
			fsize += 4 - (fsize & 3);

		// check filename and data
		char* filename = (char*)(addr+1);
		char* data = filename + nsize;
		if(strcmp(filename,"TRAILER!!!") == 0) {
			printf("cat: %s: No such file or directory\n", target);
			break;
		}
		if(strcmp(filename,target) == 0) {
			for(int i=0; i<fsize; i++){
				uart_send(data[i]);
			}
			break;
		}
		addr=(cpio_t*)(data+fsize);
	}
}

void load_cpio(cpio_t* addr, char* target){
	while(1){
		unsigned long nsize, fsize;
		nsize=strtoi(addr->c_namesize);
		fsize=strtoi(addr->c_filesize);

		// total size of the fixed header plus pathname is a multiple of four
		if((sizeof(cpio_t) + nsize) & 3)
			nsize += 4 - ((sizeof(cpio_t) + nsize) & 3);
		if(fsize & 3)
			fsize += 4 - (fsize & 3);

		// check filename and data
		char* filename = (char*)(addr+1);
		char* data = filename + nsize;
		if(strcmp(filename,"TRAILER!!!") == 0) {
			printf("load: %s: No such file or directory\n", target);
			break;
		}
		if(strcmp(filename,target) == 0) {
			char *load_addr = (char*)0x7000000;
			for(int i=0; i<fsize; i++){
				load_addr[i] = data[i];
			}
			// jump to the new kernel.
			// asm volatile("mov x0, 0x3c0  \n"); //disable core timer interrupt (11 1100 0000)
			asm volatile("mov x0, 0x340		\n"); // enable core timer interrupt (11 0100 0000)
            asm volatile("msr spsr_el1, x0  \n");
            asm volatile("msr elr_el1, %0   \n" :: "r"(0x7000000));
            asm volatile("msr sp_el0, %0    \n" :: "r"(0x7000000));

			// enable the core timer’s interrupt
			set_time(2);
			enable_timer_interrupt();


            asm volatile("eret              \n");

			break;
		}
		addr=(cpio_t*)(data+fsize);
	}
}

void getName(char* target){
	uart_puts("Filename: ");
	int buffer_counter = 0;
	while (1) {
		target[buffer_counter] = async_uart_getc();
		buffer_counter = parse(target[buffer_counter], buffer_counter);

		if (target[buffer_counter] == '\n') {
			// Enter
			target[buffer_counter] = 0;
			break;
		}
	}
}

void cat_file(){
	cpio_t* addr = (cpio_t*)cpio_start;//qemu: 0x8000000 ,raspi: 0x20000000
	if (strcmp((char*)(addr+1), "."))
		printf("error no cpio\n");
	else {
		char target[128];
		getName(target);

		cat_cpio(addr,target);
	}
}

void list_file(){
	cpio_t* addr = (cpio_t*)cpio_start;//qemu: 0x8000000 ,raspi: 0x20000000
	if (strcmp((char*)(addr+1), "."))
		printf("error no cpio\n");
	else
		while(1){
			unsigned long nsize, fsize;
			nsize=strtoi(addr->c_namesize);
			fsize=strtoi(addr->c_filesize);

			// total size of the fixed header plus pathname is a multiple of four
			if((sizeof(cpio_t) + nsize) & 3)
				nsize += 4 - ((sizeof(cpio_t) + nsize) & 3);
			if(fsize & 3)
				fsize += 4 - (fsize & 3);


			// check filename and data
			char* filename = (char*)(addr+1);
			char* data = filename + nsize;
			if(strcmp(filename,"TRAILER!!!") == 0)
				break;

			printf(filename);
			uart_send('\n');

			addr=(cpio_t*)(data+fsize);
		}
}

void load_file(){
	cpio_t* addr = (cpio_t*)cpio_start;//qemu: 0x8000000 ,raspi: 0x20000000
	if (strcmp((char*)(addr+1), "."))
		printf("error no cpio\n");
	else {
		char target[128];
		getName(target);

		load_cpio(addr,target);
	}
}