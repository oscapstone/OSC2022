#include "mini_uart.h"
#include "shell.h"
#include "stringUtils.h"
#include "reboot.h"

#define MAX_BUFFER_SIZE 256u
static char input_buffer[MAX_BUFFER_SIZE];

void getCommand();
void parseCommand();

void shell(void) {
    while (1) {
        uart_send_string("> ");
        getCommand();
        parseCommand();
    }
}

void getCommand() {
    unsigned int index = 0;
    char recv;
    while (1) {
        recv = uart_recv();
        if (recv == '\r')
            continue;
        uart_send(recv);
        input_buffer[index++] = recv;
        index = index < MAX_BUFFER_SIZE ? index : MAX_BUFFER_SIZE - 1;
        if (recv == '\n') {
            input_buffer[index - 1] = '\0';
            break;
        }
    }
}

void parseCommand() {
    if (compareString(input_buffer, "\0") == 0)
        uart_send_string("\r");
    else if (compareString(input_buffer, "hello") == 0)
        uart_send_string("\rHello World!\r\n");
    else if (compareString(input_buffer, "reboot") == 0) {
        uart_send_string("\rrebooting ...\r\n");
        reboot(100);
        while (1) {}
    }
    else if (compareString(input_buffer, "help") == 0) {
        uart_send_string("\rhelp               : print this help menu\r\n");
        uart_send_string("hello              : print Hello World!\r\n");
        uart_send_string("reboot             : reboot the device\r\n");
    }
    else
        uart_send_string("\rcommand not found!\r\n");
}