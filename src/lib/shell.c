#include "shell.h"

void print_system_info () {
    unsigned int board_revision;
    unsigned int board_serial_msb, board_serial_lsb;
    unsigned int mem_base;
    unsigned int mem_size;

    get_board_revision(&board_revision);
    uart_puts("Board revision   : 0x");
    uart_puth(board_revision);

    get_board_serial(&board_serial_msb, &board_serial_lsb);
    uart_puts("\nBoard serial     : 0x");
    uart_puth(board_serial_msb);
    uart_puth(board_serial_lsb);

    get_memory_info(&mem_base, &mem_size);
    uart_puts("\nMem base address : 0x");
    uart_puth(mem_base);
    uart_puts("\nMem size         : 0x");
    uart_puth(mem_size);

    return;
}

void welcome () {
    uart_puts("[OSC 2022] Author : ydlee\n\r");
    uart_puts("-------------------------------------\n\r");
}

void help () {
    uart_puts("-------------------------------------\n\r");
    uart_puts("* clear   : clear the screen.\n\r");
    uart_puts("* help    : print help menu.\n\r");
    uart_puts("* hello   : print Hello World!\n\r");
    uart_puts("* sysinfo : print system infomations.\n\r");
    uart_puts("* reboot  : reboot the device.\n\r");
    uart_puts("-------------------------------------");
    return;
}

void clear () {
    uart_putc(0x0C);
    return;
}

//-----------------------------------------------------------------

void echo_back (char c) {

    uart_putc(c);

    if (c == '\n')
    {
        uart_putc('\r');
    }
    else if (c == '\b')
    {
        uart_putc(' ');
        uart_putc('\b');
    }

    return;
}

void read_cmd (char *cmd) {

    char c;
    int pos = 0;

    uart_puts("> ");

    do {

        c = uart_get();
        
        /* Deal with special operation */
        if (c == '\n') 
        {
            echo_back(c);
            cmd[pos] = '\0';
            break;
        } 
        else if (c == '\b') 
        {
            if (pos > 0)
            {
                echo_back(c);
                cmd[pos] = '\0';
                pos--;
            }
        } 
        else if (pos >= 0 && pos < 255) 
        {
            echo_back(c);
            cmd[pos] = c;
            pos++;
        }

    } while (1);

    return;
}

void do_cmd (char *cmd) {

    if ( strcmp(cmd, "clear") == 0 ) 
    {
        clear();
        return;
    }
    else if ( strcmp(cmd, "help") == 0 ) 
    {
        help();
    }
    else if ( strcmp(cmd, "hello") == 0 ) 
    {
        uart_puts("Hello World!");
    }
    else if ( strcmp(cmd, "sysinfo") == 0 ) 
    {
        print_system_info();
    }
    else if ( strcmp(cmd, "reboot") == 0 ) 
    {
        reset(5);
    } 
    else 
    {
        uart_puts("invalid command.");
    }

    uart_puts("\n\r");

    return;
}

void shell_start () {

    clear();
    welcome();
    char cmd[256];

    while (1) {
        memset(cmd, 0, sizeof(cmd));
        read_cmd(cmd);
        do_cmd(cmd);
    }

}