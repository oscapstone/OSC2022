#include "shell.h"
#include "uart.h"
#include "utils.h"
#include "my_string.h"
#include "mbox.h"

void boot_msg()
{
    uart_printf("\n\nWELCOME !!!!!\n\n");
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
    uart_printf("\n\n");
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
    if (strcmp(cmd, "help") == 0)
        help();
    else if (strcmp(cmd, "hello") == 0)
        hello();
    else if (strcmp(cmd, "reboot") == 0)
        reboot(100);
    else if (strcmp(cmd, "mailbox") == 0)
        mailbox();
    else
        uart_printf("%s: command not found\n", cmd);
}

void help()
{
    uart_printf("help\t: print this help menu\n");
    uart_printf("hello\t: print hello world\n");
    uart_printf("reboot\t: reboot the device\n");
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