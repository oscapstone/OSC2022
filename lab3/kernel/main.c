#include "uart.h"
#include "string.h"
#include "power.h"
#include "mbox.h"
#include "cpio.h"
#include "stdint.h"
// #include "device_tree.h"
#define RAMFS_ADDR 0x8000000

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
    uart_puts("user\t: load and run a user program in the initramfs\n");    
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
    uint64_t cur_addr = RAMFS_ADDR;
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
        uart_puts(file_name);
        uart_puts("\r\n");

        cur_addr = (uint64_t)((cur_addr + name_size + 3) & (~3));
        cur_addr = (uint64_t)((cur_addr + file_size + 3) & (~3));
    }
}

void cpio_load_user_program(char *target_program, uint64_t target_addr) {
    uint64_t cur_addr = RAMFS_ADDR;
    cpio_newc_header* cpio_ptr;
    uint64_t name_size, file_size;
    char *file_name;

    while (1) {
        cpio_ptr = (cpio_newc_header *)cur_addr;
        name_size = hex_to_int64(cpio_ptr->c_namesize);
        file_size = hex_to_int64(cpio_ptr->c_filesize);


        cur_addr += sizeof(cpio_newc_header);
        file_name = (char *)cur_addr;
        // the end is indicated by a special record with pathname "TRAILER!!!"
        if (strcmp(file_name, "TRAILER!!!") == 0) break;

        cur_addr = (uint64_t)((cur_addr + name_size + 3) & (~3));
        if (strcmp(file_name, target_program) == 0) {
            char *file_content = (char *)cur_addr;
            char *target_content = (char *)target_addr;
            for (unsigned long long i = 0; i < file_size; i++) {
                target_content[i] = file_content[i];
            }
            return;
        }
        cur_addr = (uint64_t)((cur_addr + file_size + 3) & (~3));
    }
    uart_puts("No such file\n");
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
    uint64_t cur_addr = RAMFS_ADDR;
    cpio_newc_header* cpio_ptr;
    uint64_t name_size, file_size;
    char *file_name;

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
       
        cur_addr = (uint64_t)((cur_addr + name_size + 3) & (~3));
        if(!strcmp(file_name, file)){
            char *file_content = (char *)cur_addr;
            for(uint64_t i=0; i<file_size; i++){
                if(file_content[i] == '\n')
                    uart_send('\r');
                uart_send(file_content[i]);
            }
            uart_puts("\r\n");
            break;
        }
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

void command_load_user_program() {
  uint64_t spsr_el1 = 0x3c0; // EL0t with interrupt disabled
  uint64_t target_addr = 0x30000000;
  uint64_t target_sp = 0x31000000;
  cpio_load_user_program("user_program.img", target_addr);
  asm volatile("msr spsr_el1, %0" : : "r"(spsr_el1));
  asm volatile("msr elr_el1, %0" : : "r"(target_addr));
  asm volatile("msr sp_el0, %0" : : "r"(target_sp));
  asm volatile("eret");
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
    else if ( !strcmp(buffer, "user")) command_load_user_program();
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
