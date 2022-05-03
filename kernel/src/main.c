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

extern Thread *thread_head;

int main(unsigned long dtb_base){
    uart_init();
    // uart_getc();
    print_string(UITOHEX, "[*] DTB_BASE: 0x", dtb_base, 1);
    fdt_traverse((fdt_header *)dtb_base, initramfs_callback);
    all_allocator_init();    
    init_thread_pool_and_head();
    init_task_head();

    enable_timer_irq();
    // enable_AUX_MU_IER_r();
    enable_irq(); // DAIF set to 0b0000


    Thread *new_thread = thread_create(print_string);
    print_string(UITOHEX, "new_thread->ustack: ", (unsigned long long )new_thread->ustack_addr, 1);
    print_string(UITOHEX, "new_thread->kstack: ", (unsigned long long )new_thread->kstack_addr, 1);
    print_string(UITOHEX, "thead_head->next: ", (unsigned long long )thread_head->list.next, 1);


    PrintWelcome();
    ShellLoop();
    
    return 0;
}