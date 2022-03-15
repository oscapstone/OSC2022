#include "mini_uart.h"
#include "shell.h"
#include "utils.h"
#include "reboot.h"
#include "peripherals/mail_box.h"
#include "mail_box.h"
#include "initrd.h"

char buffer[MAX_BUFFER_SIZE];

void get_command();
void parse_command();

void get_board_revision();

void shell(void) {
    while (1) {
        uart_send_string("> ");
        get_command();
        parse_command();
    }
}

void get_command() {
    unsigned int index = 0;
    char recv;
    while (1) {
        recv = uart_recv();
        if (recv == '\r')
            continue;
        uart_send(recv);
        buffer[index++] = recv;
        index = index < MAX_BUFFER_SIZE ? index : MAX_BUFFER_SIZE - 1;
        if (recv == '\n') {
            buffer[index - 1] = '\0';
            break;
        }
    }
}

void parse_command() {
    if (compare_string(buffer, "\0") == 0)
        uart_send_string("\r");
    else if (compare_string(buffer, "hello") == 0)
        uart_send_string("\rHello World!\r\n");
    else if (compare_string(buffer, "reboot") == 0) {
        uart_send_string("\rrebooting ...\r\n");
        reboot(100);
        while (1) {}
    }
    else if (compare_string(buffer, "info") == 0) {
        uart_send_string("\r");
        get_board_revision();
        get_arm_memory();
    }
    else if (compare_string(buffer, "ls") == 0) {
        uart_send_string("\r");
        initrd_list();
    }
    else if (compare_string(buffer, "cat") == 0)
    {
        uart_send_string("Filename: ");
        get_command();
        initrd_cat(buffer);
    }
    else if (compare_string(buffer, "help") == 0) {
        uart_send_string("\rhelp               : print this help menu\r\n");
        uart_send_string("hello              : print Hello World!\r\n");
        uart_send_string("reboot             : reboot the device\r\n");
        uart_send_string("info               : print device info\r\n");
        uart_send_string("ls                 : print files in rootfs\r\n");
    }
    else
        uart_send_string("\rcommand not found!\r\n");
}
