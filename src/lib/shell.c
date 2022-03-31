#include "shell.h"

void print_sys_info() {
    unsigned int revision;
    unsigned int mem_base;
    unsigned int mem_size;

    get_board_revision(& revision);
    uart_puts("\rboard revision: 0x");
    uart_hex(revision);

    get_arm_memory(& mem_base, &mem_size);
    uart_puts("\nmemory base: ");
    uart_hex(mem_base);
    uart_puts("\nmemory size: ");
    uart_hex(mem_size);
    uart_puts("\n");
}

void welcome_msg() {
    char *welcome = "\rwelcome to my shell, type 'help' for more details!\n";
    uart_puts(welcome);
}

void helper() {
    uart_puts("*************************************************\n");
    uart_puts("* help       : print this help menu\n");
    uart_puts("* hello      : print hello world!\n");
    uart_puts("* reboot     : reboot the device\n");
    uart_puts("* hwinfo     : print the hardware information\n");
    uart_puts("* ls         : print files in cpio archieve\n");
    uart_puts("* cat        : print content in cpio archieve\n");
    uart_puts("* exec       : execute user program\n");
    uart_puts("* core time  : print seconds after booting\n");
    uart_puts("* async send : async print message\n");
    uart_puts("* async getc : print the hex ascii we get\n");
    uart_puts("*************************************************\n");
}

// handle all commands
void cmd_handler(char *cmd) {
    uart_puts("\r");
    if (strcmp(cmd, "help") == 0) {
        helper();
    }
    else if (strcmp(cmd, "hello") == 0) {
        uart_puts("Hello World!\n");
    }
    else if (strcmp(cmd, "reboot") == 0) {
        uart_puts("rebooting................\n");
        reset(100);
    }
    else if (strcmp(cmd, "\0") == 0) {
        uart_puts("\r");
    }
    else if (strcmp(cmd, "hwinfo") == 0) {
        print_sys_info();
    }
    else if (strcmp(cmd, "ls") == 0) {
        cpio_ls((cpio_new_header *)CPIO_BASE);
    }
    else if (strcmp(cmd, "cat") == 0) {
        uart_puts("Filename: ");
        char input[MAX_BUFFER_SIZE];
        cmd_reader(input);
        uart_puts("\r\n");
        cpio_cat((cpio_new_header *)CPIO_BASE, input);
    }
    else if (strcmp(cmd, "exec") == 0) {
        uart_puts("Filename: ");
        char input[MAX_BUFFER_SIZE];
        cmd_reader(input);
        uart_puts("\r\n");
        exec((cpio_new_header *)CPIO_BASE, input, 0);
    }
    else if (strcmp(cmd, "core time") == 0) {
        exec((cpio_new_header *)CPIO_BASE, "user_program.img", 1);
    }
    else if (strcmp(cmd, "async send") == 0) {
        uart_async_puts("async send finish!");
        uart_puts("\r\n");
    }
    else if (strcmp(cmd, "async getc") == 0) {
        char c = uart_async_getc();
        uart_puts("the ascii of char we get: 0x");
        uart_hex(c);
        uart_puts("\r\n");
    }
    else {
        uart_puts("invalid command!");
    }
    uart_puts("\n");
    return;
}

// read command
void cmd_reader(char *cmd) {
    unsigned int idx = 0;
    char c;
    while (1) {
        c = uart_getc();
        if (c == '\r') continue;
        uart_send(c);
        cmd[idx++] = c;
        idx = idx < MAX_BUFFER_SIZE ? idx : MAX_BUFFER_SIZE - 1;
        if (c == '\n') {
            cmd[idx - 1] = '\0';
            break;
        }
    }
}

// do shell
void exe_shell() {
    print_sys_info();
    welcome_msg();
    char cmd[MAX_BUFFER_SIZE];

    while (1) {
        uart_puts(">>");
        cmd_reader(cmd);
        cmd_handler(cmd);
    }
}