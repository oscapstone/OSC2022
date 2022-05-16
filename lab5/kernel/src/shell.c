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
#include "printf.h"
#include "dtb.h"

void clean_buffer(char * buffer, int buffer_len)
{
    for(int i = 0 ; i < buffer_len ; i++)
        buffer[i] = '\0';
}

void command_help()
{
    uart_puts("This is kernel shell\n");
    uart_puts("help\t\t: print this help menu\n");
    uart_puts("hello\t\t: print Hello World!\n");
    uart_puts("reboot\t\t: reboot the device\n");
    uart_puts("mailbox\t\t: show information through mailbox\n");
    uart_puts("dtb\t\t: parse device tree message\n");
    uart_puts("dtb all\t\t: parse device tree message\n");
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
    cpio_ls();
}


void command_cat(char* pathname){
    cpio_cat(pathname);
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
    if (mbox_call(MBOX_CH_PROP,mbox)) {
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
    if (mbox_call(MBOX_CH_PROP,mbox)) {
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
    if (mbox_call(MBOX_CH_PROP,mbox)) {
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

void command_load_user_program(const char *program_name) {
    uint64_t spsr_el1 = 0x0; // EL0t with interrupt enabled
    uint64_t target_addr = 0x30100000;
    uint64_t target_sp = 0x10007030;
    cpio_load_user_program(program_name, target_addr);
    // core_timer_enable();
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
void command_thread_test1() { 
    thread_test1(); 
}
void command_thread_test2() {
    thread_test2(); 
}
void command_thread_test3() {
    thread_test3(); 
}

void command_dtb_print(int all) { dtb_print(all); }

void parse_command(char * buffer)
{
    if ( !strcmp(buffer, "help")) command_help();
    else if ( !strcmp(buffer, "hello")) command_hello();
    else if ( !strcmp(buffer, "mailbox")) command_mailbox();
    else if ( !strcmp(buffer, "ls")) command_ls();
    else if ( !strncmp(buffer, "cat", 3)) command_cat(&buffer[4]);
    else if ( !strcmp(buffer, "test")) command_test();
    else if ( !strcmp(buffer, "reboot")) reset();
    else if ( !strncmp(buffer, "run", 3)) command_load_user_program(&buffer[4]);
    else if ( !strcmp(buffer, "puts")) uart_async_puts("Test Message!\n");
    else if ( !strcmp(buffer, "timer")) core_timer_enable();
    else if ( !strncmp(buffer, "setTimeout", 10)) command_set_timeout(&buffer[11]);
    else if ( !strcmp(buffer, "buddy test")) command_buddy_test();
    else if ( !strcmp(buffer, "dma test")) command_dma_test();
    else if ( !strcmp(buffer, "dtb"))  command_dtb_print(0);
    else if ( !strcmp(buffer, "dtb all"))  command_dtb_print(1);
    else if ( !strcmp(buffer, "t1")) command_thread_test1();
    else if ( !strcmp(buffer, "t2")) command_thread_test2();
    else if ( !strcmp(buffer, "t3")) command_thread_test3();
    else command_not_found(buffer);
}

void run_shell()
{
    char buffer[64]={'\0'};
    int buffer_len=0;
    //clean buffer
    clean_buffer(buffer, 64);
    // echo everything back
    while(1) {
        printf("# ");
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
