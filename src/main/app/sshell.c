#include "uart.h"
#include "type.h"
#include "string.h"
#include "mailbox.h"
#include "reboot.h"
#include "cpio.h"
#include "heap.h"
#include "fdt.h"

static char recv_buf[100];


void init();
void cat(char* filename);
void _cmd_help();
void _cmd_hello();
void cmd_excute(char* cmd);
void read_cmd_echo(int echo);
void read_cmd_echo_callback(int echo, void (*cb)(char*) );

void read_dtb();
void read_cmd();
void run_shell();

extern uint32_t __dtb_start_addr_; //* The address of dtb file


int main(void) {

    // init_uart(270);
    init();
    run_shell();

    return 0;
}


void init() {

    init_uart(270);
    _heap_init();
}

void cat(char* filename) {
    cpio_newc_ptr_t ptr = cpio_find(filename);

    char* data = ptr->data;
    for(int i=0;i<ptr->filesize;i++) {
        uart_send(*data++);
    }
    uart_write("\n");
}


void _cmd_help() {
    uart_write("help\t:\tprint this help menu\n");
    uart_write("hello\t:\tprint Hello World!\n");
    uart_write("bv\t:\tprint the board revision\n");
    uart_write("arm_mem\t:\tprint the arm memory information\n");
    uart_write("reboot\t:\treboot in ten ticks\n");
    uart_write("loadimg\t:\tload img file into 0x80000\n");
    uart_write("ls\t:\tlist files in initramdisk\n");
    uart_write("cat\t:\tprint file content\n");
}

void _cmd_hello() {
    uart_write("Hello World!\r\n");
}

void cmd_excute(char* cmd) {
    if(strcmp(cmd, "help") == 0) {
        _cmd_help();
    } else if(strcmp(cmd, "hello") == 0) {
        _cmd_hello();
    } else if(strcmp(cmd, "bv") == 0) {
        get_board_revision();
    } else if(strcmp(cmd, "arm_mem") == 0) {
        get_arm_memory();
    } else if(strcmp(cmd, "reboot") == 0) {
        reset(10);
    } else if(strcmp(cmd, "ls") == 0) {
        cpio_list();
    } else if(strcmp(cmd, "cat") == 0) {
        uart_write("Filename: ");
        read_cmd_echo_callback(1, cat);
    } else if(strcmp(cmd, "read_dtb") == 0) {
        read_dtb();
    }else {
        uart_write("Error: No such command\r\n");
    }
}


void read_cmd_echo(int echo) {
    char c;
    char* cur = recv_buf;
    memset(recv_buf, '\0', 100);
    while(1) {
        c = uart_recv();
        uart_send(c);
        if(c == '\n') {
            *cur = '\0';
            uart_send('\r');
            cmd_excute(recv_buf);
            break;
        }
        *cur++ = c;
    }
}

void read_cmd_echo_callback(int echo, void (*cb)(char*)) {

    char c;
    char* cur = recv_buf;
    memset(recv_buf, '\0', 100);
    while(1) {
        c = uart_recv();
        uart_send(c);
        if(c == '\n') {
            *cur = '\0';
            uart_send('\r');
            break;
        }
        *cur++ = c;
    }
    cb(recv_buf);



}


void read_cmd() {
    read_cmd_echo(1);
}


void run_shell() {

    char prompt = '>';
    char space = ' ';
    while(1) {
        uart_send(prompt);
        uart_send(space);
        read_cmd();
    }
}


void read_dtb() {

    fdt_struct_t* fdt_tree = hmalloc(sizeof(fdt_struct_t));
    fdt_parse((uint32_t*)__dtb_start_addr_, fdt_tree, hmalloc);
    uart_write("Magic :");
    uart_hex(fdt_tree->header->magic);

}
