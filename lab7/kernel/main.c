#include "types.h"
#include "peripherals/mini_uart.h"
#include "init/kernel_init.h"
#include "debug/debug.h"
#include "kernel/shell.h"
#include "kernel/sched/sched.h"
#include "kernel/sched/kthread.h"

void kernel_main_thread(void){
    for(uint32_t i = 0 ; i < 10 ; i++){
        INFO("create kthread kthread_test");
//        kthread_create(kthread_test);
    }
    INFO("create kthread simple_shell");
    kthread_create(simple_shell);
}

void kernel_main(void *dtb){
	dtb += UPPER_ADDR_SPACE_BASE;
    kernel_init(dtb);
	kthread_init();
    DEBUG_KERNEL_START();
    
    INFO("create kernel main thread");
    kthread_create(kernel_main_thread);

	kthread_idle();
}

