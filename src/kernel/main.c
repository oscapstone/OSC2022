#include <stdint.h>
#include <stddef.h>
#include <uart.h>
#include <string.h>
#include <mmio.h>
#include <utils.h>
#include <kmalloc.h>
#include <initrd.h>
#include <dtb.h>
#include <exception.h>
#include <interrupt.h>
#include <timer.h>

#define PM_PASSWORD 0x5a000000

int test[0x10];

extern uint32_t _bss_start;
extern uint32_t _bss_end;
extern uint32_t _stack_end;
extern uint32_t _userspace_start;
extern uint32_t _userspace_end;

void main();

void _init()
{
    for(uint32_t* addr = &_bss_start; addr!=&_bss_end; addr++){
        *addr = 0;
    }
    main();
}

void reset(int tick) {                 // reboot after watchdog timer expire
    mmio_set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    mmio_set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    mmio_set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    mmio_set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}

void help()
{
    uart_puts("help         : print this help menu");
    uart_puts("hello        : print Hello World!");
    uart_puts("ls           : list files in initrd.");
    uart_puts("cat          : read a file.");
    uart_puts("run <file>   : execute a file.");
    uart_puts("setTimeout   : set a timer.");
    uart_puts("reboot       : reboot the device");
}

struct dtb_print_data{
    int depth;
};

int dtb_print_callback(char* node, const char *name, int depth, void *data)
{
    for(int i=0;i<depth;i++) uart_print(" ");
    uart_puts(name);
    fdt_prop *prop;
    while(prop = fdt_getnextprop(node+1, &node)){
        for(int i=0;i<depth;i++) uart_print(" ");
        uart_print("- ");
        uart_print(prop->name);
        uart_print(" len: 0x");
        uart_print_hex(prop->len);
        uart_print(" value[:4]: 0x");
        uart_putshex(fdt32_ld((uint32_t*)prop->value));
    }
    return 0;
}

void dtb_print()
{
    struct dtb_print_data data;
    data.depth = 0;
    fdt_traverse(dtb_print_callback, (void *)&data);
}

void exec(const char *filename)
{
    INITRD_FILE* file = initrd_get(filename);
    memcpy(&_userspace_start, file->filecontent, file->filesize);
    coretimer_el0_enable();
    asm("msr spsr_el1, %0"::"r"((uint64_t)0x0)); // 0x0 enable all interrupt
    asm("msr elr_el1, %0"::"r"(&_userspace_start));
    asm("msr sp_el0, %0"::"r"(&_userspace_end));
    asm("eret");
}

void print_boottime(void *arg)
{
    uint32_t cntfrq_el0;
    uint64_t cntpct_el0;
    asm("mrs %0, cntfrq_el0":"=r"(cntfrq_el0));
    asm("mrs %0, cntpct_el0":"=r"(cntpct_el0));
    uint64_t sec_from_boot = cntpct_el0 / cntfrq_el0;
    uart_print("Seconds From Booting: 0x");
    uart_putshex(sec_from_boot);
    add_timer(10, &print_boottime, 0);
}

void Timeout(void *arg)
{
    uart_print("Timeout: ");
    uart_puts((char *)arg);
}

void setTimeout()
{
    int time;
    char *msg = (char *)kmalloc(0x20);
    char time_inp[0x20];
    uart_print("Time (secs): ");
    uart_gets(time_inp);
    time = atoi(time_inp);
    uart_print("Msg: ");
    uart_gets(msg);
    add_timer(time, &Timeout, (void *)msg);
}

void main()
{
    char buf[0x100];
    uart_init();

    uart_puts("Boot!!!");
    //uart_puts("Shell");
    unsigned int board_revision;
    void* arm_memory_base;
    unsigned int arm_memory_size;

    get_board_revision(&board_revision);
    get_ARM_memory_info(&arm_memory_base, &arm_memory_size);

    u322hex(board_revision, buf, 0x10);
    uart_print("Board Revision: 0x");
    uart_puts(buf);

    u642hex((unsigned long long)arm_memory_base, buf, 0x10);
    uart_print("ARM Memory Base: 0x");
    uart_puts(buf);
    
    u322hex(arm_memory_size, buf, 0x10);
    uart_print("ARM Memory Size: 0x");
    uart_puts(buf);

    uart_print("DTB Loaded Address: 0x");
    uart_putshex((uint64_t)_DTB_ADDRESS);

    void *test_kmalloc_simple1 = kmalloc(1);
    void *test_kmalloc_simple2 = kmalloc(16);
    u642hex((uint64_t)test_kmalloc_simple1, buf, 0x10);
    uart_print("Test Simple Allocator 1: 0x");
    uart_puts(buf);
    u642hex((uint64_t)test_kmalloc_simple2, buf, 0x10);
    uart_print("Test Simple Allocator 2: 0x");
    uart_puts(buf);

    exception_vector_table_init();
    coretimer_el0_enable();
    interrupt_enable();

    uint64_t spsr_el1=0;
    asm("mrs %0, spsr_el1":"=r"(spsr_el1));
    uart_print("spsr_el1: 0x");
    uart_putshex(spsr_el1);
    add_timer(10, &print_boottime, 0);

    while(1){
        uart_print("# ");
        uart_gets(buf);
        //uart_puts(buf);
        if(strncmp(buf, "help", 4) == 0){
            help();
            //help();
        }
        else if(strncmp(buf, "hello", 5) == 0){
            uart_puts("Hello World!");
        }
        else if(strncmp(buf, "ls", 2) == 0){
            uart_puts("Parsing files...");
            INITRD_FILE* list_head = initrd_list();
            uart_puts("Parsed!!");
            while(list_head){
                uart_puts(list_head->filename);
                list_head = list_head->nextfile;
            }
        }
        else if(strncmp(buf, "cat", 3) == 0){
            uart_print("Filename: ");
            uart_gets(buf);
            INITRD_FILE* file = initrd_get(buf);
            if(!file){
                uart_puts("File not found!!!");
            }
            else{
                uart_write(file->filecontent, file->filesize);
            }
        }
        else if(strncmp(buf, "dtb", 3) == 0){
            dtb_print();
        }
        else if(strncmp(buf, "run", 3) == 0){
            exec(&buf[4]);
        }
        else if(strncmp(buf, "setTimeout", 10) == 0){
            //exec(&buf[4]);
            setTimeout();
        }
        else if(strncmp(buf, "reboot", 5) == 0){
            uart_puts("reboot!!");
            reset(1<<16);
        }
    }
    uart_write(buf, 5);
    //uart_write("Hello, world!", 13);
}