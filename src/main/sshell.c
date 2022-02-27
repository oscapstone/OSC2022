#include "uart.h"
#include "type.h"
#include "string.h"
#include "mailbox.h"
#include "reboot.h"

static char recv_buf[100];

void _cmd_help() {
    uart_write("help\t:\tprint this help menu\n");
    uart_write("hello\t:\tprint Hello World!\n");
    uart_write("bv\t:\tprint the board revision\n");
    uart_write("arm_mem\t:\tprint the arm memory information\n");
    uart_write("reboot\t:\treboot in ten ticks\n");
}

void _cmd_hello() {
    uart_write("Hello World!\n");
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
    } else {
        uart_write("Error: No such command\n");
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
            uart_send('\r');
            *cur = '\0';
            cmd_excute(recv_buf);
            break;
        }
        *cur++ = c;
    }
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

int main(void) {

    init_uart();
    run_shell();

    return 0;
}