#include "../include/cpio.h"

unsigned long CPIO_BASE;

void cpio_ls(){
	UART_write('\r');
	Info info;
	cpio_newc_header *header = (cpio_newc_header*)CPIO_BASE;

	while(1){
		parse_header(header, &info);
		if(Strncmp(info.name, "TRAILER!!!", 10) == 0)
			break;
		for(int i=0;i<info.namesize;i++)
			UART_write(*(info.name+i));
		UART_put("\r\n");

		header = info.next_header;
	}
	return;
}

void cpio_cat(){
	UART_write('\r');
	Info info;
	cpio_newc_header *header = (cpio_newc_header*)CPIO_BASE;

	UART_put("Filename: ");
	char pathname[100];
	for(int i=0;i<100;i++)
		pathname[i] = 0;

	char c;
	int i=0; 
	do{
		c = UART_read();
		if(c <= 127){
			UART_write(c);
			pathname[i++] = c;
		}
	} while(c!='\n' && c!='\r');

	//UART_put(pathname);
	UART_write('\r');
	while(1){
		parse_header(header, &info);
		//UART_put("Parsed");
		if(Strncmp(info.name, "TRAILER!!!", 10)==0 || Strncmp(info.name, pathname, info.namesize-1)==0)
			break;

		header = info.next_header;
	}

	if(Strncmp(info.name, pathname, info.namesize-1)==0){
		//UART_put("Entered");
		for(int i=0;i<info.filesize;i++)
			UART_write(*(info.file+i));
		UART_put("\r\n");
	}
	else
		UART_put("No such files\n");
}

void parse_header(cpio_newc_header* header, Info *info){
	//for(int i=0;i<6;i++)
	//	UART_write(*(header->c_magic+i));
	//UART_put(header->c_magic);
	//UART_put("\r\n");
	if(Strncmp(header->c_magic, "070701", 6) != 0){
		//UART_put(header->c_magic);
		return;
	}

	ull name_size = hex_to_dec(header->c_namesize, 8);
	//UART_write(name_size+'0');
	ull file_size = hex_to_dec(header->c_filesize, 8);
	ull header_size = sizeof(cpio_newc_header)+name_size;

	/*UART_hex(cpio_aligned(header_size));
	UART_write(' ');
	UART_hex(cpio_aligned(file_size));
	UART_write(' ');*/
	info->name = ((char*)header)+sizeof(cpio_newc_header);
	info->file = (char*)header+cpio_aligned(header_size);
	info->next_header = ((cpio_newc_header*)(info->file+cpio_aligned(file_size)));
	info->filesize = file_size;
	info->namesize = name_size;

	//UART_put("test");
	//UART_put(info->name);

	return;
}

ull cpio_aligned(ull size){
	ull tmp = size&0x03;    // last 2 bits
	size -= tmp;

	return tmp>0?size+4:size;
}

