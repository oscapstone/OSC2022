#include "uart.h"
#include "string.h"

#define CPIO_ADDRESS 0x8000000
#define HEADER_SIZE 110

extern void from_el1_to_el0(void);
extern void core_timer_enable(void);

struct cpio_newc_header {
    char c_magic[6];
    char c_ino[8];
    char c_mode[8];
    char c_uid[8];
    char c_gid[8];
    char c_nlink[8];
    char c_mtime[8];
    char c_filesize[8];
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];
    char c_check[8];
};

void exception_entry() {
    unsigned long spsr_el1, elr_el1, esr_el1;

    uart_init();

    asm volatile("mrs %0, SPSR_EL1"
                 : "=r"(spsr_el1));
    asm volatile("mrs %0, ELR_EL1"
                 : "=r"(elr_el1));
    asm volatile("mrs %0, ESR_EL1"
                 : "=r"(esr_el1));

    uart_puts("SPSR_EL1 = ");
    uart_hex(spsr_el1);
    uart_puts("\n");
    uart_puts("ELR_EL1 = ");
    uart_hex(elr_el1);
    uart_puts("\n");
    uart_puts("ESR_EL1 = ");
    uart_hex(esr_el1);
    uart_puts("\n");
    uart_puts("\n");
}

void core_timer_entry() {
    unsigned long cntfrq_el0, cntpct_el0;

    uart_init();
    asm volatile("mrs %0, CNTFRQ_EL0"
                 : "=r"(cntfrq_el0));
    asm volatile("mrs %0, cntpct_el0"
                 : "=r"(cntpct_el0));

    uart_puts("Core timer: ");
    uart_hex(cntpct_el0 / cntfrq_el0);
    uart_puts("\n");

    asm volatile("mrs x0, cntfrq_el0");
    asm volatile("lsl x0, x0, #1");
    asm volatile("msr cntp_tval_el0, x0");
    asm volatile("mov x0, 0");
    asm volatile("msr spsr_el1, x0");
}

int hex_to_int(char *c) {
    int ans = 0;
    for (int i = 0; i < 8; i++) {
        int temp = 0;
        if (c[i] >= 'A' && c[i] <= 'F')
            temp = c[i] - 'A' + 10;
        else
            temp = c[i] - '0';
        for (int j = 0; j < 7 - i; j++)
            temp *= 16;
        ans += temp;
    }
    return ans;
}

void load() {
    char *cur = (char *)(CPIO_ADDRESS);
    while (1) {
        struct cpio_newc_header *header = (struct cpio_newc_header *)(cur);
        cur += HEADER_SIZE;
        char *name = (char *)(cur);
        if (strcmp(name, "TRAILER!!!") == 0) {
            uart_puts("File not found!\n");
            break;
        }
        int namesize = hex_to_int(header->c_namesize);
        cur += namesize;
        if ((HEADER_SIZE + namesize) % 4 != 0)
            cur += 4 - (HEADER_SIZE + namesize) % 4;
        int filesize = hex_to_int(header->c_filesize);
        if (strcmp("user_program.img", name) == 0) {
            volatile unsigned char *prog = (unsigned char *)0x90000;
            for (int size = 0; size < filesize; size++)
                prog[size] = *(cur + size);
            break;
        }
        cur += filesize;
        if (filesize % 4 != 0)
            cur += 4 - filesize % 4;
    }
}

void get_uart_input(char *input) {
    char tmp;
    int i = 0;
    while (1) {
        tmp = uart_getc();
        if (tmp == '\n') {
            uart_puts("\n");
            input[i] = '\0';
            break;
        } else
            uart_send(tmp);

        input[i] = tmp;
        i++;
    }
    return;
}

void main() {

    uart_init();
    from_el1_to_el0();

    while (1) {
        uart_puts("\r# ");

        char command[1024];
        get_uart_input(command);

        uart_puts("Error command!\n");
    }
}
