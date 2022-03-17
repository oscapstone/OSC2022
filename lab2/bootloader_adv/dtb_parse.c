
#include "dtb_parse.h"
#include "uart.h"
#include "string.h"
void parse_dtb(){
    unsigned long* header_addr_ptr = dtb_addr;
    fdt_header* header = ((fdt_header*)(*header_addr_ptr));
    unsigned long struct_addr = (unsigned long)*dtb_addr + convert_big_to_small_endian(header->off_dt_struct);
    unsigned long string_addr = (unsigned long)*dtb_addr + convert_big_to_small_endian(header->off_dt_strings);
    unsigned long struct_end_addr = struct_addr + convert_big_to_small_endian(header->size_dt_struct);
    unsigned int *struct_ptr = (unsigned int*)struct_addr;
	
    unsigned int token, token_len, nameoff, len;
    char *node_name, *property_val, *property_name;
	
    while((unsigned long)struct_ptr < struct_end_addr){
        token = convert_big_to_small_endian(*struct_ptr);
        struct_ptr += 1;

        if(token == 1){ //FDT_BEGIN_NODE
            node_name = (char*)struct_ptr;
            uart_puts("###########################################\r\n");
            uart_puts(node_name);
            uart_puts("\r\n");
            token_len = strlen(node_name);
            struct_ptr += token_len/4;
            struct_ptr += (token_len%4 ? 1 : 0);
        }
        else if(token == 2){//FDT_END_NODE
            uart_puts("###########################################\r\n");
        }
        else if(token == 3){//FDT_PROP
            len = convert_big_to_small_endian(*struct_ptr);
            struct_ptr += 1;
            nameoff = convert_big_to_small_endian(*struct_ptr);
            struct_ptr += 1;
        
            if(len != 0){   // Not Empty Property
                property_name = (char*)string_addr+nameoff;
                property_val = (char*)struct_ptr;
                struct_ptr += len/4;
                struct_ptr += (len%4 ? 1 : 0);
                uart_puts("Len: ");
                uart_put_int(len);
                uart_puts("\r\n");
                uart_puts("property Name: ");
                uart_puts(property_name);
                uart_puts("\r\n");
                uart_puts("property Value: ");
                uart_puts(property_val);
				uart_puts("\r\n");
                uart_puts("******************************************\r\n");
            }
        }
        else if(token == 4){//FDT_NOP
            ;
        }
        else if(token == 9){//FDT_END
            break;
        }
    } 
}

void parse_header(){
    unsigned long* header_addr_ptr = dtb_addr;
    fdt_header* header_addr = ((fdt_header*)(*header_addr_ptr));
    uart_puts("address: ");
    uart_put_int((unsigned int)(*header_addr_ptr));
    uart_puts("\n");
    uart_puts("totalsize: ");
    uart_put_int(convert_big_to_small_endian(header_addr->totalsize));
    uart_puts("\n");
    uart_puts("offset in bytes of the structure block: \t");
    uart_put_int(convert_big_to_small_endian(header_addr->off_dt_struct));
    uart_puts("\n");
    uart_puts("offset in bytes of the strings block: \t");
    uart_put_int(convert_big_to_small_endian(header_addr->off_dt_strings));
    uart_puts("\n");
    uart_puts("offset in bytes of the memory reservation block: \t");
    uart_put_int(convert_big_to_small_endian(header_addr->off_mem_rsvmap));
    uart_puts("\n");
    uart_puts("length in bytes of the strings block section : \t");
    uart_put_int(convert_big_to_small_endian(header_addr->size_dt_strings));
    uart_puts("\n");
    uart_puts("length in bytes of the structure block: \t");
    uart_put_int(convert_big_to_small_endian(header_addr->size_dt_struct));
    uart_puts("\n");
}

unsigned int convert_big_to_small_endian(unsigned int BE_num){
    int size = 4;
    unsigned int LE_num = 0;
    for(int i=0;i<size;i++){
        LE_num <<= 8;
        LE_num = LE_num + ((BE_num>>(8*i))&0xff);
    }
    return LE_num;
}
