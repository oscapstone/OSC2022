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
    uart_puts("   ____   _____  _____    ___   ___ ___  ___     /\\      /\\    __         _ _         __  \n");
    uart_puts("  / __ \\ / ____|/ ____|  |__ \\ / _ \\__ \\|__ \\   |/\\|    |/\\|  / /        | | |        \\ \\ \n");
    uart_puts(" | |  | | (___ | |          ) | | | | ) |  ) |      _   _    | |_   _  __| | | ___  ___| |\n");
    uart_puts(" | |  | |\\___ \\| |         / /| | | |/ /  / /      | | | |   | | | | |/ _` | |/ _ \\/ _ \\ |\n");
    uart_puts(" | |__| |____) | |____    / /_| |_| / /_ / /_      | |_| |   | | |_| | (_| | |  __/  __/ |\n");
    uart_puts("  \\____/|_____/ \\_____|  |____|\\___/____|____|      \\__,_|   | |\\__, |\\__,_|_|\\___|\\___| |\n");
    uart_puts("                                                              \\_\\__/ |                /_/ \n");
    uart_puts("                                                                |___/                     \n");
    uart_puts("------------------------------------------------------------------------------------------\n");
    uart_puts("[Welcom to simple shell, type 'help' for a list of commands]\n");
    return;
}

void help () {
    uart_puts("-------------------------------------\n");
    uart_puts("* clear   : clear the screen.\n");
    uart_puts("* help    : print help menu.\n");
    uart_puts("* hello   : print Hello World!\n");
    uart_puts("* sysinfo : print system infomations.\n");
    uart_puts("* reboot  : reboot the device.\n");
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
    int pos  = 0;
    int tail = 0;
    int tmp  = 0;

    uart_puts("> ");
    memset(cmd, 0, sizeof(cmd));

    do {

        c = uart_get();
        
        /* Deal with special operation */
        if (c == '\n') 
        {
            echo_back(c);
            break;
        } 
        else if (c == '\b') 
        {
            if (pos > 0)
            {
                if (pos < tail)
                {
                    strpullout(cmd, pos - 1);
                    tmp = tail - pos;
                    uart_putc('\b');
                    for (int i = 0; i < tmp; i++) uart_putc(cmd[pos - 1 + i]);
                    uart_putc(' ');
                    for (int i = 0; i < tmp + 1; i++) uart_putc('\b');
                }
                else
                {
                    echo_back(c);
                    cmd[pos - 1] = '\0';
                }
                pos--;
                tail--;
            }
        } 
        else if (c == 0x1B)
        {
            /* ESCAPE */
            c = uart_get();

            if (c == 0x5B) 
            {
                c = uart_get();
                /* Deal with up, down, left, right */
                if (c == 0x41) 
                {
                    /* UP */
                }
                else if (c == 0x42)
                {
                    /* DOWN */
                }
                else if (c == 0x44)
                {
                    /* LEFT */
                    if (pos > 0) 
                    {
                        uart_putc(0x1B);
                        uart_putc(0x5B);
                        uart_putc(0x44);
                        pos--;
                    }
                }
                else if (c == 0x43)
                {
                    /* RIGHT */
                    if (pos < tail) 
                    {
                        uart_putc(0x1B);
                        uart_putc(0x5B);
                        uart_putc(0x43);
                        pos++;
                    }
                }
                else
                {
                    /* not special operation */
                    uart_putc(0x5B);
                    uart_putc(c);
                }
            }
            else
            {
                uart_puts("[assert] unknown input.\n");
            }
        }
        else if (pos >= 0 && pos < 256) 
        {
            if (pos < tail)
            {
                strinsert(cmd, c, pos);
                tmp = tail - pos;
                for (int i = 0; i < tmp + 1; i++) uart_putc(cmd[pos + i]);
                for (int i = 0; i < tmp; i++) uart_putc('\b');
            }
            else
            {
                echo_back(c);
                cmd[pos] = c;
            }
            pos++;
            tail++;
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

    uart_puts("\n");

    return;
}

void shell_start () {

    clear();
    welcome();
    char cmd[256];

    while (1) {
        read_cmd(cmd);
        do_cmd(cmd);
    }

}