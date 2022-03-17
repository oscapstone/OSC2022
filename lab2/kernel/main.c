#include "uart.h"
#include "string.h"
#include "power.h"
#include "mbox.h"
#include "str_tool.h"
#include "stdint.h"
#include "device_tree.h"

void clean_buffer(char * buffer, int buffer_len)
{
    for(int i = 0 ; i < buffer_len ; i++)
        buffer[i] = '\0';
}

void command_help()
{
    uart_puts("\n");
    uart_puts("help\t: print this help menu\n");
    uart_puts("hello\t: print Hello World!\n");
    uart_puts("reboot\t: reboot the device\n");
    uart_puts("mailbox\t: show information through mailbox\n");
    uart_puts("ls\t: show all files\n");
    uart_puts("cat\t: show file info\n");
    uart_puts("test\t: test simple allocator\n");
    uart_puts("dbt\t: dbt info\n");
    uart_puts("parsedbt\t: parse dbt info\n");
}

void command_hello()
{
    uart_puts("\n");
    uart_puts("Hello World!\n");
}

void command_not_found(char * buffer)
{
    uart_puts("\n");
    uart_puts("command *");
    uart_puts(buffer);
    uart_puts("* not exist\n");
}

void command_ls(){
    uint64_t cur_addr = 0x2000000;
    cpio_newc_header* cpio_ptr;
    uint64_t name_size, file_size;
    char *file_name;
    // char *file_content;
    uart_puts("\r");

    while(1){
        cpio_ptr = (cpio_newc_header*)cur_addr;
        name_size = hex_to_int64(cpio_ptr->c_namesize);
        file_size = hex_to_int64(cpio_ptr->c_filesize);

        cur_addr += sizeof(cpio_newc_header);
        file_name = (char*)cur_addr;
        if(!strcmp(file_name, "TRAILER!!!"))
            break;            

        // file_content = file_name + name_size;
        // uart_puts("File Name: ");
        uart_puts(file_name);
        uart_puts("\r\n");

        // for(uint64_t i=0; i<file_size; i++){
        //     if(file_content[i] == '\n')
        //         uart_send('\r');
        //     uart_send(file_content[i]);
        // }
            
        // uart_puts(file_content);
        // uart_puts("\r\n");
        // uart_puts("File Size: ");
        // uart_puts(itoa(file_size, 10));
        // uart_puts(" bytes");    
        // uart_puts("\r\n");
        // uart_puts("==========\r\n");

        cur_addr = (uint64_t)((cur_addr + name_size + 3) & (~3));
        cur_addr = (uint64_t)((cur_addr + file_size + 3) & (~3));
    }

}

void command_cat(){
    uart_send('\r');
    uart_puts("Filename: ");
    char file[64]={'\0'};
    int file_idx=0;
    while(1){
        char c = uart_getc();
        if(c=='\n') {
            uart_send('\r');
            uart_send(c);
            break;
        }
        uart_send(c);
        file[file_idx++] =c;
    }
    uint64_t cur_addr = 0x2000000;
    cpio_newc_header* cpio_ptr;
    uint64_t name_size, file_size;
    char *file_name;
    char *file_content;

    while(1){
        cpio_ptr = (cpio_newc_header*)cur_addr;
        name_size = hex_to_int64(cpio_ptr->c_namesize);
        file_size = hex_to_int64(cpio_ptr->c_filesize);

        cur_addr += sizeof(cpio_newc_header);
        file_name = (char*)cur_addr;
        if(!strcmp(file_name, "TRAILER!!!")){
            uart_puts("The file is not exist\r\n");
            break;            
        }
        file_content = file_name + name_size;
        if(!strcmp(file_name, file)){
            for(uint64_t i=0; i<file_size; i++){
                if(file_content[i] == '\n')
                    uart_send('\r');
                uart_send(file_content[i]);
            }
            uart_puts("\r\n");
            break;
        }

        cur_addr = (uint64_t)((cur_addr + name_size + 3) & (~3));
        cur_addr = (uint64_t)((cur_addr + file_size + 3) & (~3));
    }

}

void command_mailbox()
{
    // get serail number
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETSERIAL;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;
    uart_puts("\n");
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("serial number is: ");
        uart_hex(mbox[6]);
        uart_hex(mbox[5]);
        uart_puts("\n");
    }
    // get board revision
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETBDVS;     // get board revision
    mbox[3] = 4;                    // buffer size
    mbox[4] = 4;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("board revision is: ");
        uart_hex(mbox[6]);
        uart_hex(mbox[5]);
        uart_puts("\n");
    }

    // get arm memory
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETARMMEM;   // get arm memory info
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("arm base addr: ");
        uart_hex(mbox[5]);
        uart_puts("\n");
        uart_puts("arm addr size: ");
        uart_hex(mbox[6]);
        uart_puts("\n");
    }
}

uint64_t allocate_end = 0x6000000;
void* simple_allocate(int size)
{
    uint64_t location = allocate_end;
    allocate_end += size;
    uart_puts("allocate memory at:");
    uart_hex(location);
    uart_puts(", with size:");
    uart_hex(size);
    uart_puts("\r\n");

    return (void *)location;
}

void command_test()
{
    // test simple_allocate
    uart_puts("\r");
    uart_puts("test simple_allocate\n");
    char * c = simple_allocate(30);
    c[0]='a';
    c[1]='b';
    c[2]='\0';
    uart_puts("c:");
    uart_puts(c);
    uart_puts("\r\n");
    char * d = simple_allocate(30);
    d[0]='x';
    d[1]='y';
    d[2]='\0';
    uart_puts("d:");
    uart_puts(d);
    uart_puts("\r\n");
}

void parse_command(char * buffer)
{
    if ( !strcmp(buffer, "help")) command_help();
    else if ( !strcmp(buffer, "hello")) command_hello();
    else if ( !strcmp(buffer, "mailbox")) command_mailbox();
    else if ( !strcmp(buffer, "ls")) command_ls();
    else if ( !strcmp(buffer, "cat")) command_cat();
    else if ( !strcmp(buffer, "test")) command_test();
    else if ( !strcmp(buffer, "reboot")) reset();
    else if ( !strcmp(buffer, "dbt")) print_dt_info();
    else if ( !strcmp(buffer, "parsedbt")) parse_dt();
    else command_not_found(buffer);
}


void main()
{
    // set up serial console
    uart_init();

    char buffer[64]={'\0'};
    int buffer_len=0;
    //clean buffer
    clean_buffer(buffer, 64);

    // echo everything back
    while(1) {
        uart_send('#');
        while(1){
            char c = uart_getc();
            uart_send(c);
            if(c=='\n'){
                //parse buffer
                parse_command(buffer);
                //clean buffer
                clean_buffer(buffer, 64);
                buffer_len = 0;
                break;
            }
            buffer[buffer_len++] = c;
        }
    }
}
