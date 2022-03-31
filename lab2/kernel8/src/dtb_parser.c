#include "dtb_parser.h"
void* INITRD_ADDR;

unsigned int get_be_uint32(void *ptr) {
    unsigned char *bytes = (unsigned char *)ptr;
    return bytes[3] | bytes[2] << 8 | bytes[1] << 16 | bytes[0] << 24;
}

unsigned int convert_big_to_small_endian(unsigned int num){
    return (num>>24&0xff) | (num<<8&0xff0000) | (num>>8&0xff00) | (num<<24&0xff000000);
}

int find_cpio(char* str){
    char *ptr = str;
    while(*(ptr+5)!=0){
        if(ptr[0]=='-' && ptr[1]=='s' && ptr[2]=='t' && ptr[3]=='a'&& ptr[4]=='r' && ptr[5]=='t'){
            return 1;
        }
        ptr+=1;
    }
    return 0;
}

void print_dt_info(){
    unsigned long addr = DT_ADDR;
    unsigned long *dt_addr = (unsigned long*)addr;
    struct fdt_header* header = (struct fdt_header*)*dt_addr;
    unsigned int totalsize, off_dt_struct, off_dt_strings, size_dt_strings, size_dt_struct;
    totalsize = convert_big_to_small_endian(header->totalsize);
    off_dt_struct = convert_big_to_small_endian(header->off_dt_struct);
    off_dt_strings = convert_big_to_small_endian(header->off_dt_strings);
    size_dt_strings = convert_big_to_small_endian(header->size_dt_strings);
    size_dt_struct = convert_big_to_small_endian(header->size_dt_struct);

    uart_puts("###########################################\r\n");
    uart_puts("Device Tree Address \t");
    uart_put_int(*dt_addr);
    uart_puts("\r\n");
    uart_puts("Total Size \t\t");
    uart_put_int(totalsize);
    uart_puts("\r\n");
    uart_puts("off_dt_struct \t\t");
    uart_put_int(off_dt_struct);
    uart_puts("\r\n");
    uart_puts("off_dt_strings \t\t");
    uart_put_int(off_dt_strings);
    uart_puts("\r\n");
    uart_puts("size_dt_struct \t\t");
    uart_put_int(size_dt_struct);
    uart_puts("\r\n");
    uart_puts("size_dt_strings \t");
    uart_put_int(size_dt_strings);
    uart_puts("\r\n");
    uart_puts("###########################################\r\n");
}

void init_dtb(dtb_callback_t callback){
    unsigned long addr = DT_ADDR;
    unsigned long *dt_addr = (unsigned long*)addr;
    struct fdt_header* header = (struct fdt_header*)*dt_addr;

    unsigned int off_dt_struct, off_dt_strings, size_dt_struct;
    off_dt_struct = convert_big_to_small_endian(header->off_dt_struct);
    off_dt_strings = convert_big_to_small_endian(header->off_dt_strings);
    size_dt_struct = convert_big_to_small_endian(header->size_dt_struct);
    unsigned long struct_addr = (unsigned long)*dt_addr + off_dt_struct;
    unsigned long string_addr = (unsigned long)*dt_addr + off_dt_strings;
    unsigned long struct_end_addr = struct_addr + size_dt_struct;
    unsigned int *struct_ptr = (unsigned int*)struct_addr;
	
    unsigned int cur_val, cur_len, cur_nameoff;
    char *cur_name, *cur_name_val;
	
    while((unsigned long)struct_ptr < struct_end_addr){
        cur_val = convert_big_to_small_endian(*struct_ptr);
        struct_ptr += 1;

        if(cur_val == FDT_BEGIN_NODE){
            cur_name = (char*)struct_ptr;
            cur_len = strlen(cur_name);
            struct_ptr += cur_len/4;
            struct_ptr += (cur_len%4 ? 1 : 0);
        }
        else if(cur_val == FDT_END_NODE){
        }
        else if(cur_val == FDT_PROP){
            cur_len = convert_big_to_small_endian(*struct_ptr);
            struct_ptr += 1;
            cur_nameoff = convert_big_to_small_endian(*struct_ptr);
            struct_ptr += 1;
        
            if(cur_len != 0){   // Not Empty Property
                cur_name = (char*)string_addr+cur_nameoff;
                cur_name_val = (char*)struct_ptr;
                struct_ptr += cur_len/4;
                struct_ptr += (cur_len%4 ? 1 : 0);
                 if(find_cpio(cur_name)){
                    //  INITRD_ADDR = get_be_uint32(cur_name_val);
                    callback(cur_name_val);
                    uart_puts("Len: ");
                    uart_put_int(cur_len);
                    uart_puts("\r\n");
                    uart_puts("Cur Name: ");
                    uart_puts(cur_name);
                    uart_puts("\r\n");
                    uart_puts("Cur Value: ");
                    uart_hex(cur_name_val);
                    uart_puts("\r\n");
                    uart_hex(get_be_uint32(cur_name_val));
                    uart_puts("\r\n");
                    uart_puts("******************************************\r\n");
                 }
            }
        }
        else if(cur_val == FDT_NOP){
            ;
        }
        else if(cur_val == FDT_END){
            break;
        }
		else if(cur_val == PADDING){
            ;
        }
        else{
            uart_puts("ERROR! Unknown Node Value!\r\n");
            uart_put_int(cur_val);
            uart_puts("\r\n");
            return;
        }
    }
}

void parse_dt(){
    unsigned long addr = DT_ADDR;
    unsigned long *dt_addr = (unsigned long*)addr;
    struct fdt_header* header = (struct fdt_header*)*dt_addr;

    unsigned int off_dt_struct, off_dt_strings, size_dt_struct;
    off_dt_struct = convert_big_to_small_endian(header->off_dt_struct);
    off_dt_strings = convert_big_to_small_endian(header->off_dt_strings);
    size_dt_struct = convert_big_to_small_endian(header->size_dt_struct);
    unsigned long struct_addr = (unsigned long)*dt_addr + off_dt_struct;
    unsigned long string_addr = (unsigned long)*dt_addr + off_dt_strings;
    unsigned long struct_end_addr = struct_addr + size_dt_struct;
    unsigned int *struct_ptr = (unsigned int*)struct_addr;
	
    unsigned int cur_val, cur_len, cur_nameoff;
    char *cur_name, *cur_name_val;
	
    while((unsigned long)struct_ptr < struct_end_addr){
        cur_val = convert_big_to_small_endian(*struct_ptr);
        struct_ptr += 1;

        if(cur_val == FDT_BEGIN_NODE){
            cur_name = (char*)struct_ptr;
            uart_puts("###########################################\r\n");
            uart_puts(cur_name);
            uart_puts("\r\n");
            cur_len = strlen(cur_name);
            struct_ptr += cur_len/4;
            struct_ptr += (cur_len%4 ? 1 : 0);
        }
        else if(cur_val == FDT_END_NODE){
            uart_puts("###########################################\r\n");
        }
        else if(cur_val == FDT_PROP){
            cur_len = convert_big_to_small_endian(*struct_ptr);
            struct_ptr += 1;
            cur_nameoff = convert_big_to_small_endian(*struct_ptr);
            struct_ptr += 1;
        
            if(cur_len != 0){   // Not Empty Property
                cur_name = (char*)string_addr+cur_nameoff;
                cur_name_val = (char*)struct_ptr;
                struct_ptr += cur_len/4;
                struct_ptr += (cur_len%4 ? 1 : 0);
                uart_puts("Len: ");
                uart_put_int(cur_len);
                uart_puts("\r\n");
                uart_puts("Cur Name: ");
                uart_puts(cur_name);
                uart_puts("\r\n");
                uart_puts("Cur Value: ");
                uart_hex(get_be_uint32(cur_name_val));
                uart_puts("\r\n");
                uart_puts("******************************************\r\n");
            }
        }
        else if(cur_val == FDT_NOP){
            ;
        }
        else if(cur_val == FDT_END){
            break;
        }
		else if(cur_val == PADDING){
            ;
        }
        else{
            uart_puts("ERROR! Unknown Node Value!\r\n");
            uart_put_int(cur_val);
            uart_puts("\r\n");
            return;
        }
    }
}

void save_cpio_addr(char *ptr){
    INITRD_ADDR = get_be_uint32(ptr);
}

