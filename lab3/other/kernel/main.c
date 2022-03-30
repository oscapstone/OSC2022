#include "uart.h"
#include "string.h"

#define CPIO_ADDRESS 0x8000000
#define CORE0_IRQ_SOURCE 0x40000060
#define HEADER_SIZE 110
#define MESSAGE_QUEUE 0x100000
#define MESSAGE_INSERT 0x120000

extern void from_el1_to_el0(void);
extern void core_timer_enable(void);

int message_count = 0;

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

struct message {
    char content[1024];
    unsigned long seconds;
};

int malloc_address = 0x9000000;

void *simple_malloc(int size) {
    int cur = malloc_address;
    malloc_address = malloc_address + size;
    return cur;
}

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
}

unsigned long char_to_ulong(char *c) {
    unsigned long ret = 0, i = 0;
    while (*(c + i) != '\0') {
        ret = ret * 10 + *(c + i) - '0';
        i++;
    }
    return ret;
}

void queue_timer_handler() {
    struct message *q = (struct message *)MESSAGE_QUEUE;
    uart_puts(q->content);
    uart_puts("\n");
    for (int i = 1; i < message_count; i++) {
        struct message *next = (struct message *)(q + i);
        struct message *now = (struct message *)(q + (i - 1));
        strcpy(now->content, next->content);
        now->seconds = next->seconds;
    }
    message_count--;
    // uart_uint(message_count);
    // uart_puts("\n");
    if (message_count == 0) {
        asm volatile("mov x0, 0");
        asm volatile("msr cntp_ctl_el0, x0"); // disable
        return;
    }
    unsigned long cntpct_el0;
    asm volatile("mrs %0, cntpct_el0"
                 : "=r"(cntpct_el0));
    asm volatile("msr cntp_tval_el0, %0"
                 :
                 : "r"(q->seconds - cntpct_el0));
}

void sort_message(int size) {
    struct message *q = (struct message *)MESSAGE_QUEUE;
    struct message *ins = (struct message *)MESSAGE_INSERT;
    int i;
    for (i = 0; i < size; i++) {
        if ((q + i)->seconds > ins->seconds) {
            for (int j = size + 1; j > i; j--) {
                struct message *next = (struct message *)(q + j);
                struct message *now = (struct message *)(q + (j - 1));
                strcpy(next->content, now->content);
                next->seconds = now->seconds;
            }
            break;
        }
    }
    struct message *now = (struct message *)(q + i);
    strcpy(now->content, ins->content);
    now->seconds = ins->seconds;
}

unsigned long cstr_to_ulong(char *s) {
    unsigned long ret = 0;
    while (*s != '\0') {
        ret *= 10;
        ret += (*s - '0');
        s++;
    }
    return ret;
}

void main() {

    uart_init();
    struct message *queue_head = (struct message *)MESSAGE_QUEUE;
    char command[1024];

    while (1) {
        uart_puts("\r# ");

        get_uart_input(command);

        if (strcmp(command, "load") == 0) {
            load();
            from_el1_to_el0();
            asm volatile("mov x1, #0x90000");
            asm volatile("br x1");
        } else if (strcmp(command, "st") == 0) {
            uart_puts("here is st\n");
            struct message *msg = (struct message *)MESSAGE_INSERT;
            uart_puts("MESSAGE: ");
            get_uart_input(command);
            strcpy(msg->content, command);
            uart_puts("SECONDS: ");
            get_uart_input(command);
            asm volatile("msr DAIFSet, 0xf");
            unsigned long cntpct_el0, cntfrq_el0;
            asm volatile("mrs %0, cntfrq_el0"
                         : "=r"(cntfrq_el0));
            asm volatile("mrs %0, cntpct_el0"
                         : "=r"(cntpct_el0));
            msg->seconds = cntpct_el0 + cstr_to_ulong(command) * cntfrq_el0;
            sort_message(message_count);
            message_count++;

            asm volatile("mov x0, 1");
            asm volatile("msr cntp_ctl_el0, x0"); // disable
            asm volatile("mrs %0, cntpct_el0"
                         : "=r"(cntpct_el0));
            asm volatile("msr cntp_tval_el0, %0"
                         :
                         : "r"(queue_head->seconds - cntpct_el0));
            asm volatile("msr DAIFClr, 0xf");
        } else
            uart_puts("Error command!\n");
    }
}
