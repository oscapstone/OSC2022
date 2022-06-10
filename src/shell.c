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
    uart_puts("run                : run user program\n");
    uart_puts("balloc <order>     : buddy alloc test\n");
    uart_puts("bfree              : buddy free test\n");
    uart_puts("malloc <order>     : mm alloc test\n");
    uart_puts("mfree              : mm free test\n");

    uart_puts("\n");
}

void jump2usr_prog(cpio_fp_t *fp)
{
    uint64_t spsr_value, sp_value;
    size_t prog_size = ascii2int(fp->header->c_filesize, 8);
    char *prog_addr = simple_alloc(prog_size);

    copy_prog_from_cpio(prog_addr, fp->data, prog_size);
    spsr_value = 0x340;
    sp_value = simple_alloc(0x8000);
    sp_value += 0x8000;
    disable_recieve_irq();

    asm volatile ("msr spsr_el1, %0" : : "r"(spsr_value));
    asm volatile ("msr elr_el1, %0"  : : "r"(prog_addr));
    asm volatile ("msr sp_el0, %0"   : : "r"(sp_value));
    asm volatile ("eret");
}

void copy_prog_from_cpio(char *dst_addr, const char *src_addr, size_t prog_size)
{
    memcpy(dst_addr, src_addr, prog_size);
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

void svc_handler()
{
    uint32_t value;
    asm volatile ("mrs %0, spsr_el1": "=r"(value));
    sync_uart_puts("spsr_el1: 0x");
    uart_hex(value);

    asm volatile ("mrs %0, elr_el1": "=r"(value));
    sync_uart_puts("\nelr_el1:  0x");
    uart_hex(value);

    asm volatile ("mrs %0, esr_el1": "=r"(value));
    sync_uart_puts("\nesr_el1:  0x");
    uart_hex(value);
    sync_uart_puts("\n");
}

void shell()
{
    PrintHelp();

    while (1) {
        if (read_buf_idx > 0)
            if (read_buf[read_buf_idx-1] == '\r')
                cmd_handler();

        // do something
        asm volatile("nop");
        asm volatile("nop");
    }
}

void *baddr[64];
int baddr_head = 0;
int baddr_tail = 0;

void *maddr[64];
int maddr_head = 0;
int maddr_tail = 0;

void cmd_handler()
{
    char *tok;
    cpio_fp_t fp;
    int idx;

    for (idx = 0; idx < read_buf_idx; idx++) {
        cmd_buf[idx] = read_buf[idx];
        read_buf[idx] = '\0';
    }
    
    cmd_buf[--idx] = '\0';
    read_buf_idx = 0;
    tok = strtok(cmd_buf, ' ');
    
    
    if (strcmp(tok, "help") == 0) PrintHelp();
    else if (strcmp(tok, "hello") == 0) PrintHello();
    else if (strcmp(tok, "sysinfo") == 0) PrintInfo();
    else if (strcmp(tok, "reboot") == 0) reset(100);
    else if (strcmp(tok, "clear") == 0) clear_screen();
    else if (strcmp(tok, "ls") == 0) cpio_ls();
    else if (strcmp(tok, "cat") == 0) cpio_cat(strtok(NULL, ' '));
    else if (strcmp(tok, "lshw") == 0) dtb_parse(NULL);
    else if (strcmp(tok, "alloc") == 0) alloc_test(strtok(NULL, ' '));
    else if (strcmp(tok, "run") == 0) {
        cpio_get_file_info("user_program", &fp);
        jump2usr_prog(&fp);
    }
    else if (strcmp(tok, "balloc") == 0) {
        int order = atoi(strtok(NULL, ' '));
        baddr[baddr_head] = buddy_alloc(order);
        baddr_head++;
    }
    else if (strcmp(tok, "bfree") == 0) {
        buddy_free(baddr[baddr_tail++]);
    }
    else if (strcmp(tok, "malloc") == 0) {
        int size = atoi(strtok(NULL, ' '));
        maddr[maddr_head++] = mm_alloc(size);
    }
    else if (strcmp(tok, "mfree") == 0) {
        mm_free(maddr[maddr_tail++]);
    }
    else if (strcmp(tok, "log") == 0){
        uart_sdec("baddr_head = ", baddr_head, "\n");
        uart_sdec("baddr_tail = ", baddr_tail, "\n");
    }
    else if (tok != NULL) uart_puts("Command not found\n");

    for (idx = 0; idx < CMD_BUF_SIZE; idx++)
		cmd_buf[idx] = '\0';
}