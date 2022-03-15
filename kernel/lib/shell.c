#include "shell.h"
#include "uart.h"
#include "utils.h"
#include "my_string.h"
#include "cpio.h"
#include "smalloc.h"

void clear()
{
    uart_printf("\x1b[H\x1b[J");
}

void boot_msg()
{
    clear();
    uart_printf("WELCOME !!!!!\n\n");
    uart_printf("⣿⣿⣿⣿⣿⣿⠿⢋⣥⣴⣶⣶⣶⣬⣙⠻⠟⣋⣭⣭⣭⣭⡙⠻⣿⣿⣿⣿⣿\n");
    uart_printf("⣿⣿⣿⣿⡿⢋⣴⣿⣿⠿⢟⣛⣛⣛⠿⢷⡹⣿⣿⣿⣿⣿⣿⣆⠹⣿⣿⣿⣿\n");
    uart_printf("⣿⣿⣿⡿⢁⣾⣿⣿⣴⣿⣿⣿⣿⠿⠿⠷⠥⠱⣶⣶⣶⣶⡶⠮⠤⣌⡙⢿⣿\n");
    uart_printf("⣿⡿⢛⡁⣾⣿⣿⣿⡿⢟⡫⢕⣪⡭⠥⢭⣭⣉⡂⣉⡒⣤⡭⡉⠩⣥⣰⠂⠹\n");
    uart_printf("⡟⢠⣿⣱⣿⣿⣿⣏⣛⢲⣾⣿⠃⠄⠐⠈⣿⣿⣿⣿⣿⣿⠄⠁⠃⢸⣿⣿⡧\n");
    uart_printf("⢠⣿⣿⣿⣿⣿⣿⣿⣿⣇⣊⠙⠳⠤⠤⠾⣟⠛⠍⣹⣛⣛⣢⣀⣠⣛⡯⢉⣰\n");
    uart_printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡶⠶⢒⣠⣼⣿⣿⣛⠻⠛⢛⣛⠉⣴⣿⣿\n");
    uart_printf("⣿⣿⣿⣿⣿⣿⣿⡿⢛⡛⢿⣿⣿⣶⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡈⢿⣿\n");
    uart_printf("⣿⣿⣿⣿⣿⣿⣿⠸⣿⡻⢷⣍⣛⠻⠿⠿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⢇⡘⣿\n");
    uart_printf("⣿⣿⣿⣿⣿⣿⣿⣷⣝⠻⠶⣬⣍⣛⣛⠓⠶⠶⠶⠤⠬⠭⠤⠶⠶⠞⠛⣡⣿\n");
    uart_printf("⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣶⣬⣭⣍⣙⣛⣛⣛⠛⠛⠛⠿⠿⠿⠛⣠⣿⣿\n");
    uart_printf("⣦⣈⠉⢛⠻⠿⠿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠿⠛⣁⣴⣾⣿⣿⣿⣿\n");
    uart_printf("⣿⣿⣿⣶⣮⣭⣁⣒⣒⣒⠂⠠⠬⠭⠭⠭⢀⣀⣠⣄⡘⠿⣿⣿⣿⣿⣿⣿⣿\n");
    uart_printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⡈⢿⣿⣿⣿⣿⣿\n");
    uart_printf("\n");
}

void cmd_input(char *cmd)
{
    uart_printf("\r# ");
    cmd[0] = 0;
    int idx =  0;
    char c;

    while (1)
    {
        c = uart_getc();
        if ((c >= 32 && c <= 126) || c == '\n' || c == '\r')
        {
            cmd[idx++] = (c == '\n' || c == '\r') ? '\0' : c;
            uart_send(c);
        }
        if (c == '\n' || c == '\r')
        {
            uart_printf("\r");
            if (idx > 1)
                cmd_resolve(cmd);
            uart_printf("# ");
            idx = 0;
        }
    }

}

void cmd_resolve(char *cmd)
{
    char *argv[100];
    unsigned int argc = 0;
    unsigned int cmd_length = strlen(cmd);
    int i, j;

    argv[0] = smalloc(MAX_LEN);
    for (i = 0, j = 0; i < cmd_length; ++i)
    {
        if (cmd[i] == ' ')
        {
            argv[argc][j] = '\0';
            argv[++argc] = smalloc(MAX_LEN);
            j = 0;
            continue;
        }
        argv[argc][j++] = cmd[i];
    }
    argv[argc++][j] = '\0';

    if (strcmp(argv[0], "help") == 0)
        help();
    else if (strcmp(argv[0], "hello") == 0)
        hello();
    else if (strcmp(argv[0], "reboot") == 0)
        reboot(100);
    else if (strcmp(argv[0], "mailbox") == 0)
        mailbox();
    else if (strcmp(argv[0], "clear") == 0)
        clear();
    else if (strcmp(argv[0], "ls") == 0)
        ls();
    else if (strcmp(argv[0], "cat") == 0)
        cat(argv[1]);
    else
        uart_printf("%s: command not found\n", argv[0]);
}

void help()
{
    uart_printf("help\t: print this help menu\n");
    uart_printf("hello\t: print hello world\n");
    uart_printf("reboot\t: reboot the device\n");
    uart_printf("mailbox\t: show infos of board revision and ARM memory\n");
    uart_printf("ls\t: list directory contents\n");
    uart_printf("cat\t: concatenate files and print on the standard output\n");
    uart_printf("clear\t: clear page\n");
}

void hello()
{
    uart_printf("hello world\n");
}

void reboot(int tick)
{
    uart_printf("rebooting ... \n");
    reset(tick);
    while (1);
}

void mailbox()
{
    volatile unsigned int mbox[36];
    get_board_revision(mbox);
    get_arm_memory(mbox); 
}

void ls()
{
    cpio_ls();
}

void cat(const char* name)
{
    if (cpio_cat(name) == 0)
        uart_printf("File \"%s\" not found\n", name);
}
