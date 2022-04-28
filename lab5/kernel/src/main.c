#include "uart.h"
#include "string.h"
#include "power.h"
#include "mbox.h"
#include "cpio.h"
#include "timer.h"
#include "exception.h"
#include "alloc.h"
#include "utils.h"
#include "thread.h"
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
    uart_puts("help\t\t: print this help menu\n");
    uart_puts("hello\t\t: print Hello World!\n");
    uart_puts("reboot\t\t: reboot the device\n");
    uart_puts("mailbox\t\t: show information through mailbox\n");
    uart_puts("ls\t\t: show all files\n");
    uart_puts("cat\t\t: show file info\n");
    uart_puts("test\t\t: test simple allocator\n");
    uart_puts("user\t\t: load and run a user program in the initramfs\n");    
    uart_puts("timer\t\t: core_timer_enable\n");    
    uart_puts("puts\t\t: async_puts Test Message\n");    
    uart_puts("buddy test\t: test for buddy system\n");
    uart_puts("dma test \t: test for dynamic memory allocation\n");
    uart_puts("setTimeout [MESSAGE] [SECONDS]\t: prints MESSAGE after SECONDS\n");
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

void command_cat(char* pathname){
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
        if(!strcmp(file_name, pathname)){
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

void command_test()
{
    // test malloc
    uart_puts("\r");
    uart_puts("test malloc\n");
    char * c = malloc(30);
    c[0]='a';
    c[1]='b';
    c[2]='\0';
    uart_puts("c:");
    uart_puts(c);
    uart_puts("\r\n");
    char * d = malloc(30);
    d[0]='x';
    d[1]='y';
    d[2]='\0';
    uart_puts("d:");
    uart_puts(d);
    uart_puts("\r\n");
}

void command_load_user_program() {
    uint64_t spsr_el1 = 0x0; // EL0t with interrupt enabled
    uint64_t target_addr = 0x30000000;
    uint64_t target_sp = 0x31000000;
    cpio_load_user_program("user_program.img", target_addr);
    core_timer_enable();
    asm volatile("msr spsr_el1, %0" : : "r"(spsr_el1));
    asm volatile("msr elr_el1, %0" : : "r"(target_addr));
    asm volatile("msr sp_el0, %0" : : "r"(target_sp));
    asm volatile("eret");
}

void command_set_timeout(char *args) {
  uint32_t duration = 0;
  for (int i = 0; args[i]; i++) {
    if (args[i] == ' ') {
      for (int j = i + 1; args[j]; j++) {
        duration = duration * 10 + (args[j] - '0');
      }
      args[i] = '\0';
      break;
    }
  }
  add_timer(timer_callback, args, duration);
}

void command_buddy_test() {
    buddy_test();
}
void command_dma_test() { 
    dma_test();
}
void command_thread_test() { 
    thread_test();
}
void parse_command(char * buffer)
{
    if ( !strcmp(buffer, "help")) command_help();
    else if ( !strcmp(buffer, "hello")) command_hello();
    else if ( !strcmp(buffer, "mailbox")) command_mailbox();
    else if ( !strcmp(buffer, "ls")) command_ls();
    else if ( !strncmp(buffer, "cat", 3)) command_cat(&buffer[4]);
    else if ( !strcmp(buffer, "test")) command_test();
    else if ( !strcmp(buffer, "reboot")) reset();
    else if ( !strcmp(buffer, "user")) command_load_user_program();
    else if ( !strcmp(buffer, "puts")) uart_async_puts("Test Message!\n");
    else if ( !strcmp(buffer, "timer")) core_timer_enable();
    else if ( !strncmp(buffer, "setTimeout", 10)) command_set_timeout(&buffer[11]);
    else if ( !strcmp(buffer, "buddy test")) command_buddy_test();
    else if ( !strcmp(buffer, "dma test")) command_dma_test();
    else if ( !strcmp(buffer, "t1")) command_thread_test();
    else command_not_found(buffer);
}


void main()
{
    // set up serial console
    uart_init();
    buddy_init();
    //welcome message
    uart_puts("*****************************\r\n");
    uart_puts("*       welcome OSC2022     *\r\n");
    uart_puts("*****************************\r\n");
    timeout_event_init();
    enable_interrupt();
    thread_init();
    char buffer[64]={'\0'};
    int buffer_len=0;
    //clean buffer
    clean_buffer(buffer, 64);
    // echo everything back
    while(1) {
        uart_send('#');
        while(1){
            char c = uart_async_getc();
            if(c=='\n')uart_send('\r');
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
