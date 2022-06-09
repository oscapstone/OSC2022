#include "cpio.h"

#include "devicetree.h"
#include "uart.h"
#include "utils.h"
#include "shell.h"
#include "exception.h"
#include "mem.h"
#include "thread.h"
#include "tmpfs.h"
#include "vfs.h"

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
			tmp = tmp*16 + str[i] - 'A'+10;
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

void jump_cpio(char* target){
	cpio_t* addr = (cpio_t*)cpio_start;//qemu: 0x8000000 ,raspi: 0x20000000
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
			jump_thread(data, fsize);
			break;
		}
		addr=(cpio_t*)(data+fsize);
	}
}

void load_cpio(char* target){
	cpio_t* addr = (cpio_t*)cpio_start;//qemu: 0x8000000 ,raspi: 0x20000000
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
			exec_thread(data, fsize);
			/*
			char *load_addr = (char*)0x7000000;
			for(int i=0; i<fsize; i++){
				load_addr[i] = data[i];
			}
			// jump to the new kernel.
			// asm volatile("mov x0, 0x3c0  \n"); //disable core timer interrupt (11 1100 0000)
			asm volatile("mov x0, 0x340		\n"); // enable core timer interrupt (11 0100 0000)
            asm volatile("msr spsr_el1, x0  \n");
            asm volatile("msr elr_el1, %0   \n" :: "r"((unsigned long)load_addr));
            asm volatile("msr sp_el0, %0    \n" :: "r"((unsigned long)load_addr));

			// enable the core timer’s interrupt
			set_time(1000000);
			enable_timer_interrupt();


            asm volatile("eret              \n");
			*/
			break;
		}
		addr=(cpio_t*)(data+fsize);
	}
}

// void load_cpio(cpio_t* addr, char* target){
// 	while(1){
// 		unsigned long nsize, fsize;
// 		nsize=strtoi(addr->c_namesize);
// 		fsize=strtoi(addr->c_filesize);

// 		// total size of the fixed header plus pathname is a multiple of four
// 		if((sizeof(cpio_t) + nsize) & 3)
// 			nsize += 4 - ((sizeof(cpio_t) + nsize) & 3);
// 		if(fsize & 3)
// 			fsize += 4 - (fsize & 3);

// 		// check filename and data
// 		char* filename = (char*)(addr+1);
// 		char* data = filename + nsize;
// 		if(strcmp(filename,"TRAILER!!!") == 0) {
// 			printf("load: %s: No such file or directory\n", target);
// 			break;
// 		}
// 		if(strcmp(filename,target) == 0) {
// 			char *load_addr = (char*)0x7000000;
// 			for(int i=0; i<fsize; i++){
// 				load_addr[i] = data[i];
// 			}
// 			// jump to the new kernel.
// 			// asm volatile("mov x0, 0x3c0  \n"); //disable core timer interrupt (11 1100 0000)
// 			asm volatile("mov x0, 0x340		\n"); // enable core timer interrupt (11 0100 0000)
//             asm volatile("msr spsr_el1, x0  \n");
//             asm volatile("msr elr_el1, %0   \n" :: "r"(0x7000000));
//             asm volatile("msr sp_el0, %0    \n" :: "r"(0x7000000));

// 			// enable the core timer’s interrupt
// 			set_time(1000000);
// 			enable_timer_interrupt();


//             asm volatile("eret              \n");

// 			break;
// 		}
// 		addr=(cpio_t*)(data+fsize);
// 	}
// }

void getName(char* target){
	uart_puts("Filename: ");
	int buffer_counter = 0;
	while (1) {
		#ifdef ASYNC_UART
        target[buffer_counter] = async_uart_getc();
        #else
        target[buffer_counter] = uart_getc();
        #endif
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

		// load_cpio(addr,target);
		load_cpio(target);
	}
}

void build_file_tree(vnode* root) {
	List node, dirname;
	node.entry = root;
	char *none = "";
	dirname.entry = none;
	List *top = &node;
	top->next = NULLPTR;
	List *top_name = &dirname;
	top_name->next = NULLPTR;

	cpio_t* addr = (cpio_t*)cpio_start;
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

			while (top_name->next != NULLPTR) {
				if (!strncmp(top_name->entry, filename))
					break;
				else {
					List *free_node = top;
					List *free_name = top_name;
					top = top->next;
					top_name = top_name->next;
					kfree(free_node);
					kfree(free_name);
				}
			}
			if(strcmp(filename,"TRAILER!!!") == 0)
				break;

			char *childname;
			if (strlen(top_name->entry))
				childname = filename +1 + strlen(top_name->entry);
			else
				childname = filename;

			vnode *parent = (vnode*)top->entry;
			vnode *child = vnode_create(parent, childname);
			File_Info *child_info = (File_Info*)child->internal;

			int c_mode = strtoi(addr->c_mode) >> 12;
			if (c_mode == MODE_DIR) {
				strcpy(child_info->name, childname);
				child_info->mode = MODE_DIR;
				child_info->size = 0;
				child_info->data = (vnode**)kmalloc(TMPFS_DIR_LEN * 8);
				for (int i=0; i<TMPFS_DIR_LEN; i++)
        			((vnode**)child_info->data)[i] = NULLPTR;

				if (!strcmp(child_info->name, ".")) {
					addr=(cpio_t*)(data+fsize);
					continue;
				}
				else if(!strcmp(child_info->name, "initramfs")) {
					struct mount *new_mount = kmalloc(sizeof(struct mount));
					register_filesystem("initramfs");
					struct filesystem *mount_fs = find_fs("initramfs");
					mount_fs->setup_mount(mount_fs, new_mount);
					vnode *mount_vnode = new_mount->root;

					File_Info *mount_info = (File_Info*)mount_vnode->internal;
					strcpy(mount_info->name, "initramfs");
					new_mount->root->parent = parent;

					File_Info* parent_info = (File_Info*)parent->internal;
					vnode **childs = (vnode**)parent_info->data;
					for (int i=0; i<TMPFS_DIR_LEN; i++) {
						if (childs[i] == child) {
							childs[i] = mount_vnode;
							break;
						}
					}

					child = mount_vnode;
				}

				List *new_node = kmalloc(sizeof(List));
				List *new_dir = kmalloc(sizeof(List));
				new_node->entry = child;
				new_dir->entry = filename;
				new_node->next = top;
				new_dir->next = top_name;
				top = new_node;
				top_name = new_dir;

			}
			else if (c_mode == MODE_FILE) {
				strcpy(child_info->name, childname);
				child_info->mode = MODE_FILE;
				child_info->size = fsize;
				child_info->data = data;
			}
			else {
				printf("unknown c_mode\n");
				while (1) {}
			}

			addr=(cpio_t*)(data+fsize);
		}
}