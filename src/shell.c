#include "shell.h"

void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {                 // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}

void PrintHelp()
{
    uart_puts("help               : print this help menu\n");
    uart_puts("hello              : print Hello World!\n");
    uart_puts("sysinfo            : print system information\n");
    uart_puts("reboot             : reboot the device\n");
    uart_puts("ls                 : show all files in cpio archive\n");
    uart_puts("cat <filename>     : show content stored in the file\n");
    uart_puts("lshw               : print device tree structure\n");
    uart_puts("alloc              : alloc test\n");
}

void PrintHello()
{
    uart_puts("Hello World!\n");
}

void PrintInfo()
{
    // get the board's unique serial number with a mailbox call
    mbox[0] = 7*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = 0x10002;   // get serial number command
    mbox[3] = 4;                    // buffer size
    mbox[4] = 0;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = MBOX_TAG_LAST;

    //mbox[7] = ;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("My board revision is: 0x");
        uart_hex(mbox[5]);
        uart_puts("\n");
    } else {
        uart_puts("Unable to query serial!\n");
    }

    // get the board's unique serial number with a mailbox call
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = 0x10005;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 0;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;
    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("My memory base address is: 0x");
        uart_hex(mbox[5]);
        uart_puts("\n");
        uart_puts("My memory size is: 0x");
        uart_hex(mbox[6]);
        uart_puts("\n");
    } else {
        uart_puts("Unable to query serial!\n");
    }
}

void clear_screen()
{
    uart_send(27);
    uart_puts("[2J");
    uart_send(27);
    uart_puts("[H");
}

void alloc_test(char *str_num)
{
    void *addr;
    size_t size = atoi(str_num);

    if (size == 0) return;

    addr = simple_alloc(size);
    uart_puts("allocate address: 0x");
    uart_hex(addr);
    uart_puts("\nsize is: ");
    uart_dec(size);
    uart_puts(" bytes\n");
    
}

void shell()
{
    int idx;
    char c;
    int buf_size = sizeof(cmd_buf) / 8;
    char *tok;

    dtb_parse(cpio_init);
    PrintHelp();

    for (idx = 0; idx < buf_size; idx++)
		cmd_buf[idx] = '\0';

    while (1) {

        idx = 0;

        // get the whole command
        while (1) {
            c = uart_getc();
            if (c == '\r') {
                uart_send('\n');
				uart_send('\r');
				cmd_buf[idx++] = '\0';
				break;
            }
            uart_send(c);
            cmd_buf[idx++] = c;
        }

        // parse the command
        tok = strtok(cmd_buf, ' ');
        if (strcmp(tok, "help") == 0) PrintHelp();
        else if (strcmp(tok, "hello") == 0) PrintHello();
        else if (strcmp(tok, "sysinfo") == 0) PrintInfo();
        else if (strcmp(tok, "reboot") == 0) reset(1000);
        else if (strcmp(tok, "clear") == 0) clear_screen();
        else if (strcmp(tok, "ls") == 0) {
            //if (CPIO_BASE == NULL) dtb_parse(cpio_init);
            cpio_ls();
        }
        else if (strcmp(tok, "cat") == 0) {
            //if (CPIO_BASE == NULL) dtb_parse(cpio_init);
            cpio_cat(strtok(NULL, ' '));
        }
        else if (strcmp(tok, "lshw") == 0) dtb_parse(NULL);
        else if (strcmp(tok, "alloc") == 0) alloc_test(strtok(NULL, ' '));
        else if (tok != NULL) uart_puts("Command not found\n");

        for (; idx >= 0; idx--)
		    cmd_buf[idx] = '\0';
    }
}