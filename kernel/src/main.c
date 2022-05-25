#include <uart.h>
#include <shell.h>
#include <cpio.h>
#include <malloc.h>
#include <string.h>
#include <fdt.h>
#include <irq.h>
#include <allocator.h>
#include <task.h>
#include <sched.h>
#include <syscall.h>
#include <vfs.h>
#include <test_fs.h>

extern Thread *run_thread_head;

int main(unsigned long dtb_base){
    unsigned long long tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));

    uart_init();
    // uart_getc();
    print_string(UITOHEX, "[*] DTB_BASE: 0x", dtb_base, 1);
    fdt_traverse((fdt_header *)dtb_base, initramfs_callback);
    all_allocator_init();    
    init_thread_pool_and_head();
    init_task_head();

    enable_timer_irq();
    enable_irq(); // DAIF set to 0b0000

    rootfs_init("tmpfs");

    fs_test1();
    fs_test2();
    fs_test3();

    PrintWelcome();
    ShellLoop();
    
    return 0;
}