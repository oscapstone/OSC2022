#include "command.h"

#include "uart.h"
#include "mbox.h"
#include "cpio.h"
#include "mem.h"
#include "devicetree.h"
#include "utils.h"

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void set(long addr, unsigned int value) {
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
    load_file();
}

void exec_lsfdt() {
    if (fdt_traverse(initramfs_callback))
            printf("flattened devicetree error\n");
}

void exec_testmem() {
    test_malloc();
}

void exec_checkmem() {
    printf(mem);
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
        uart_hex(mbox[5]);
        uart_puts("\n");
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
        uart_hex(mbox[5]);
        uart_puts("\n");
        uart_puts("My ARM memory size is: 0x");
        uart_hex(mbox[6]);
        uart_puts("\n");
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
        load_file();
    else if (!strcmp(command_string, "testmem"))
        exec_testmem();
    else if (!strcmp(command_string, "checkmem"))
        exec_checkmem();
    else if (!strcmp(command_string, "lsfdt"))
        exec_lsfdt();
    else if (!strcmp(command_string, "mbox_board_revision"))
        mbox_board_revision();
    else if (!strcmp(command_string, "mbox_arm_memory"))
        mbox_arm_memory();
    else
        command_not_found(command_string);
}