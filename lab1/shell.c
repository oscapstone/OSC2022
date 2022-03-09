#include "uart.h"
#include "util.h"
#include "myprint.h"
#include "mailbox.h"
#include "frame_buffer.h"


enum ANSI_ESC {
    Unknown,
    CursorForward,
    CursorBackward,
    Delete
};

enum ANSI_ESC decode_csi_key() {
    char c = uart_read();
    if (c == 'C') {
        return CursorForward;
    }
    else if (c == 'D') {
        return CursorBackward;
    }
    else if (c == '3') {
        c = uart_read();
        if (c == '~') {
            return Delete;
        }
    }
    return Unknown;
}

enum ANSI_ESC decode_ansi_escape() {
    char c = uart_read();
    if (c == '[') {
        return decode_csi_key();
    }
    return Unknown;
}

void shell_init() {
    uart_init();
    uart_flush();
    uart_puts("\nOSC2022 Lab 1 Demo\n");
    fb_init();
}

void shell_input(char* cmd) {
    uart_puts("\r# ");

    int idx = 0, end = 0, i;
    cmd[0] = '\0';
    char c;
    while ((c = uart_read()) != '\n') {
        // Decode CSI key sequences
        if (c == 27) {
            enum ANSI_ESC key = decode_ansi_escape();
            switch (key) {
                case CursorForward:
                    if (idx < end) idx++;
                    break;

                case CursorBackward:
                    if (idx > 0) idx--;
                    break;

                case Delete:
                    // left shift command
                    for (i = idx; i < end; i++) {
                        cmd[i] = cmd[i + 1];
                    }
                    cmd[--end] = '\0';
                    break;

                case Unknown:
                    uart_flush();
                    break;
            }
        }
        // CTRL-C
        else if (c == 3) {
            cmd[0] = '\0';
            break;
        }
        // Backspace
        else if (c == 8 || c == 127) {
            if (idx > 0) {
                idx--;
                // left shift command
                for (i = idx; i < end; i++) {
                    cmd[i] = cmd[i + 1];
                }
                cmd[--end] = '\0';
            }
        }
        else {
            // right shift command
            if (idx < end) {
                for (i = end; i > idx; i--) {
                    cmd[i] = cmd[i - 1];
                }
            }
            cmd[idx++] = c;
            cmd[++end] = '\0';
        }
        uart_printf("\r# %s \r\e[%dC", cmd, idx + 2);
    }

    uart_puts("\n");
}

void shell_controller(char* cmd) {
    if (!strcmp(cmd, "")) {
        return;
    }
    else if (!strcmp(cmd, "help")) {
        uart_puts("help      : print this help menu\n");
        uart_puts("hello     : print Hello World!\n");
        uart_puts("mailbox   : print hardware info\n");
        uart_puts("reboot    : reboot the device\n");
    }
    else if (!strcmp(cmd, "hello")) {
        uart_puts("Hello World!\n");
    }
    else if (!strcmp(cmd, "mailbox")) {
        mbox_board_revision();
        mbox_vc_memory();
    }
    else if (!strcmp(cmd, "reboot")) {
        uart_puts("Rpi rebooting...");
        reset();
        while (1); // hang until reboot
    }
    else {
        uart_printf("shell: command not found: %s\n", cmd);
    }
}
