#include "mini_uart.h"
#include "shell.h"
#include "utils.h"
#include "reboot.h"
#include "peripherals/mail_box.h"
#include "mail_box.h"
#include "cpio.h"
#include "timer.h"


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
    if (compare_string(buffer, "\0") == 0) {}
    else if (compare_string(buffer, "hello") == 0)
        uart_send_string("Hello World!\n");
    else if (compare_string(buffer, "reboot") == 0) {
        uart_send_string("rebooting ...\n");
        reboot(100);
        while (1) {}
    }
    else if (compare_string(buffer, "info") == 0) {
        get_board_revision();
        get_arm_memory();
    }
    else if (compare_string(buffer, "ls") == 0) {
        cpio_list();
    }
    else if (compare_string(buffer, "cat") == 0) {
        uart_send_string("Filename: \n");
        get_command();
        cpio_cat(buffer);
    }
    else if (compare_string(buffer, "load") == 0) {
        load_program();
    }
    else if (compare_string(buffer, "async_uart") == 0) {
        test_uart_async();
    }
    else if (compare_string(buffer, "test_timer") == 0) {
        test_timer();
    }
    else if (compare_string(buffer, "help") == 0) {
        uart_send_string("help               : print this help menu\n");
        uart_send_string("hello              : print Hello World!\n");
        uart_send_string("reboot             : reboot the device\n");
        uart_send_string("info               : print device info\n");
        uart_send_string("ls                 : print files in rootfs\n");
        uart_send_string("cat                : print file content\n");
        uart_send_string("load               : load user program\n");
        uart_send_string("async_uart         : test async uart\n");
        uart_send_string("test_timer         : test timer multiplexing\n");
    }
    else
        uart_send_string("\rcommand not found!\r\n");
}
