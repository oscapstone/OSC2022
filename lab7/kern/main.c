#include "peripheral/mailbox.h"
#include "kern/shell.h"
#include "kern/timer.h"
#include "kern/irq.h"
#include "kern/sched.h"
#include "kern/kio.h"
#include "kern/cpio.h"
#include "kern/mm.h"
#include "dtb.h"
#include "startup_alloc.h"
#include "syscall.h"
#include "string.h"

void hw_info() {
    unsigned int result[2];
    kputs("##########################################\n");
    get_board_revision(result);
    kprintf("Board revision:\t\t\t0x%x\n", result[0]);
    get_ARM_memory(result);
    kprintf("ARM memory base address:\t0x%x\n", result[0]);
    kprintf("ARM memory size:\t\t0x%x\n", result[1]);
    kputs("##########################################\n");
}

void dtb_init() {
    if (fdt_init() < 0) {
        kputs("dtb: Bad magic\n");
        return;
    }
    if (fdt_traverse(initramfs_callback) < 0)
        kputs("dtb: Unknown token\n");
    if (fdt_traverse(mm_callback) < 0)
        kputs("dtb: Unknown token\n");
    kputs("dtb: init success\n");
}

extern unsigned int __stack_kernel_top;

void reserve_memory() {
    kprintf("page used by startup allocator\n");
    reserved_kern_startup();
    kprintf("device tree\n");
    fdt_reserve();
    kprintf("initramfs\n");
    cpio_reserve();
    kprintf("initial kernel stack\n");
    mm_reserve((void *)PHY_2_VIRT((void *)&__stack_kernel_top - 0x2000), (void *)PHY_2_VIRT((void *)&__stack_kernel_top));
}

void user_prog() {
    exec("/initramfs/vfs1.img", 0);
    exit();
}

void idle_task() {
    while(1) {
        schedule();
    }
}

void initramfs_init() {
    mkdir("/initramfs", 0);
    mount(0, "/initramfs", "initramfs", 0, 0);
}

#include "test_func.h"

void kern_main() { 
    kio_init();
    mm_init();
    rootfs_init();
    runqueue_init();
    task_init();
    int_init();
    core_timer_enable();
    unsigned long tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
    timer_sched_latency();

    kputs("press any key to continue...");
    kscanc();
    kputs("\n");
    dtb_init();
    hw_info();

    reserve_memory();

    initramfs_init();
    
    thread_create(user_prog);
    // privilege_task_create(kill_zombies, 10);
    idle_task();
}