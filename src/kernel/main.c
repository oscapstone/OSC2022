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
#include <sched.h>
#include <process.h>
#include <error.h>

#define PM_PASSWORD 0x5a000000

int test[0x10];

extern uint32_t _bss_start;
extern uint32_t _bss_end;
extern uint32_t _stack_end;
extern uint32_t _userspace_start;
extern uint32_t _userspace_end;

void main();
void main_new();
void boot();
void kernel_shell();

void _init()
{
    for(uint32_t* addr = &_bss_start; addr!=&_bss_end; addr++){
        *addr = 0;
    }
    boot();
    thread_start(main_new);
    //thread_start(kernel_shell);
}

void reset(int tick) {                 // reboot after watchdog timer expire
    mmio_set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    mmio_set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void reboot_now() {                 // reboot after watchdog timer expire
    mmio_set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    mmio_set(PM_WDOG, PM_PASSWORD | 1<<14);  // number of watchdog tick
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
        //uart_print(" value[:4]: 0x");
        uart_print(" value:");
        for(int i=0;i<((((int32_t)prop->len-1)>>2)+1);i++){
            uart_print(" 0x");
            uart_print_hex(fdt32_ld(((uint32_t*)prop->value)+i));
        }
        uart_puts("");
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

void boot()
{
    char buf[0x100];
    kmalloc_init();
    uart_init();
    initrd_init();
    sched_init();
    process_init();

    uart_puts("Boot!!!");
    //uart_puts("Shell");
    unsigned int board_revision;
    void* arm_memory_base;
    unsigned int arm_memory_size;

    get_board_revision(&board_revision);
    get_ARM_memory_info(&arm_memory_base, &arm_memory_size);

    // u322hex(board_revision, buf, 0x10);
    // uart_print("Board Revision: 0x");
    // uart_puts(buf);

    kmsg("Board Revision: 0x%x", board_revision);

    // u642hex((unsigned long long)arm_memory_base, buf, 0x20);
    // uart_print("ARM Memory Base: 0x");
    // uart_puts(buf);

    kmsg("ARM Memory Base: 0x%x", arm_memory_base);
    
    // u322hex(arm_memory_size, buf, 0x10);
    // uart_print("ARM Memory Size: 0x");
    // uart_puts(buf);

    kmsg("ARM Memory Size: 0x%x", arm_memory_size);

    // uart_print("DTB Loaded Address: 0x");
    // uart_putshex((uint64_t)_DTB_ADDRESS);

    kmsg("DTB Loaded Address: 0x%x", (uint64_t)_DTB_ADDRESS);

    // void *test_kmalloc_simple1 = kmalloc(1);
    // void *test_kmalloc_simple2 = kmalloc(16);
    // u642hex((uint64_t)test_kmalloc_simple1, buf, 0x20);
    // uart_print("Test Simple Allocator 1: 0x");
    // uart_puts(buf);
    // u642hex((uint64_t)test_kmalloc_simple2, buf, 0x20);
    // uart_print("Test Simple Allocator 2: 0x");
    // uart_puts(buf);

    exception_vector_table_init();
    coretimer_el0_enable();
    
}

void test_thread_func(void *arg)
{
    char buf[0x10];
    uint64_t tid = thread_get_current();
    for(int i=0;i<1000000;i++){
        uart_print("Thread id: 0x");
        uart_print_hex(tid);
        uart_print(", 0x");
        uart_print_hex(i);
        uart_puts("");
        //uart_print(" input something: ");
        //uart_gets(buf);
        //uart_print("Thread id: 0x");
        //uart_print_hex(tid);
        //uart_print(" : ");
        //uart_puts(buf);
        for(int j=0;j<100000000;j++);
        schedule();
    }
}


void main_new()
{
    //reset(1<<15);
    char buf[0x10];
    uart_print("Press any key to start");
    uart_read_sync(buf, 1);

    uart_puts("kernel first thread start!!!");

    uart_puts("initramfs:");
    INITRD_FILE* list_head = initrd_list();
    //uart_puts("Parsed!!");
    while(list_head){
        uart_puts(list_head->filename);
        list_head = list_head->nextfile;
    }
    // for(int i=0;i<2;i++){
    //     thread_run(create_thread(test_thread_func, 0));
    // }
    // //create_thread(process_exec, "test_el0_exception");
    //thread_run(create_thread(process_exec, "test_loop"));
    //thread_run(create_thread(process_exec, "test_syscall"));
    //thread_run(create_thread(process_exec, "syscall.img"));
    thread_run(create_thread(process_exec, "vm.img"));
    sched_preempt();
    while(1){
        //uart_puts("Kernel thread.");
        schedule();
    }
}

void kernel_shell()
{
    char buf[0x100];
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
        schedule();
    }
    uart_write(buf, 5);
}

void main()
{
    char buf[0x100];
    kmalloc_init();
    uart_init();
    initrd_init();

    /*
    void *test_page1 = buddy_alloc(3);
    void *test_page2 = buddy_alloc(1);
    void *test_page3 = buddy_alloc(5);
    void *test_page4 = buddy_alloc(2);
    void *test_page5 = buddy_alloc(1);
    buddy_free(test_page1);
    buddy_free(test_page2);
    buddy_free(test_page3);
    buddy_free(test_page4);
    buddy_free(test_page5);

    test_page1 = buddy_alloc(3);
    test_page2 = buddy_alloc(1);
    test_page3 = buddy_alloc(5);
    test_page4 = buddy_alloc(2);
    test_page5 = buddy_alloc(1);

    void *test_kmalloc1 = kmalloc(0x18);
    void *test_kmalloc2 = kmalloc(0x23);
    void *test_kmalloc3 = kmalloc(0x128);
    void *test_kmalloc4 = kmalloc(0x220);
    void *test_kmalloc5 = kmalloc(0x30);

    kfree(test_kmalloc1);
    test_kmalloc1 = kmalloc(0x1f);
    kfree(test_kmalloc4);
    test_kmalloc4 = kmalloc(0x130);
    kfree(test_kmalloc4);
    kfree(test_kmalloc3);
    test_kmalloc3 = kmalloc(0x150);
    test_kmalloc4 = kmalloc(0x20010);
    */

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

    

    return ;

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