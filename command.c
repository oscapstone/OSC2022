#include "command.h"

#include "gpio.h"
#include "uart.h"
#include "mbox.h"
#include "cpio.h"
#include "mem.h"
#include "devicetree.h"
#include "utils.h"
#include "exception.h"
#include "mem.h"
#include "timer.h"

#define PM_PASSWORD 0x5a000000
#define PM_RSTC     ((volatile unsigned long)(MMIO_BASE+0x0010001c))
#define PM_WDOG     ((volatile unsigned long)(MMIO_BASE+0x00100024))

void set(unsigned long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {                 // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}

void exec_reboot() {
    uart_puts("Rebooting...\n");
    reset(100);
    while (1)
        ;
}

void exec_help() {
    uart_puts("help\t: print this help menu\n");
    uart_puts("hello\t: print Hello World!\n");
    uart_puts("ls\t: list all archives\n");
    uart_puts("cat\t: cat archive data\n");
    uart_puts("lsfdt\t: traverse fdt\n");
    uart_puts("load\t: load archive data\n");
    uart_puts("timeout\t: set timeout\n");
    uart_puts("reboot\t: reboot the device\n");
}

void exec_hello() {
    uart_puts("Hello World!\n");
}

void exec_ls() {
    list_file();
}

void exec_cat() {
    cat_file();
}

void exec_load() {
    load_cpio("initramfs/vfs1.img");
}

void exec_lsfdt() {
    if (fdt_traverse(initramfs_callback))
            printf("flattened devicetree error\n");
}

void exec_timeout(char *command_string) {
    timer_list* message = kmalloc(sizeof(timer_list)); 
    unsigned int duration = 0;
    int i = 8;
    int j = 0;
    while (command_string[i]) {
        message->argv[j] = command_string[i];
        i++;
        j++;
    }
    message->argv[j] = 0;
    i++;
    while (command_string[i]) {
        duration = duration*10 + (int)command_string[i] - (int)'0';
        i++;
    }
	// printf("executed time: %d.%ds\n", timer/10, timer%10);
    // printf("duration: 5\n");
    message->priority = duration;
    message->callback = cb_message;
    add_timer(message);
}

void exec_testmem() {
    test_malloc();
}

void exec_check(char *command_string) {
    // printf(mem);
    timer_list* message = kmalloc(sizeof(timer_list)); 
    int i = 6;
    int j = 0;
    while (command_string[i]) {
        message->argv[j] = command_string[i];
        i++;
        j++;
    }
    message->argv[j] = 0;
	// printf("executed time: %d.%ds\n", timer/10, timer%10);
    // printf("duration: 5\n");
    message->priority = 1;
    message->callback = cb_message_delay;
    add_timer(message);
}

void exec_testasync() {
    test_async_write();
}

void command_not_found(char* s) {
    uart_puts(s);
    uart_puts(": command not found\n");
}

void mbox_board_revision() {
// get the board's unique serial number with a mailbox call
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = 0x10002;              // get board revision
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("My board revision is: 0x");
        printf("%x\n", mbox[5]);
    } else {
        uart_puts("Unable to query serial!\n");
    }
}

void mbox_arm_memory() {
// get the board's unique serial number with a mailbox call
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = 0x10005;              // get ARM memory base address and size
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("My ARM memory base address is: 0x");
        printf("%x\n", mbox[5]);
        uart_puts("My ARM memory size is: 0x");
        printf("%x\n", mbox[6]);
    } else {
        uart_puts("Unable to query serial!\n");
    }
}

void parse_command(char* command_string) {
    if (!strcmp(command_string, ""))
        ;
    else if (!strcmp(command_string, "help"))
        exec_help();
    else if (!strcmp(command_string, "hello"))
        exec_hello();
    else if (!strcmp(command_string, "reboot"))
        exec_reboot();
    else if (!strcmp(command_string, "ls"))
        exec_ls();
    else if (!strcmp(command_string, "cat"))
        exec_cat();
    else if (!strcmp(command_string, "load"))
        exec_load();
    else if (!strcmp(command_string, "test"))
        exec_testmem();
    else if (!strcmp(command_string, "check"))
        exec_check(command_string);
    else if (!strcmp(command_string, "lsfdt"))
        exec_lsfdt();
    else if (!strcmp(command_string, "timeout"))
        exec_timeout(command_string);
    else if (!strcmp(command_string, "testasync"))
        exec_testasync();   
    else if (!strcmp(command_string, "mbox_board_revision"))
        mbox_board_revision();
    else if (!strcmp(command_string, "mbox_arm_memory"))
        mbox_arm_memory();
    else
        command_not_found(command_string);
}