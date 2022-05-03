#include "uart.h"
#include "string.h"
#include "buddy_system.h"
#include "utils.h"
#include "stdlib.h"
#include "sched.h"
#include "printf.h"
#include "irq.h"
#include "timer.h"
#include "thread.h"
#include "sys.h"
#include "mbox.h"

void foo() {
    for (int i = 0; i < 10; ++i) {
        printf("Thread id: %d %d\n", current_thread()->pid, i);
        delay(1000000);
        schedule();
    }
}

void fork_test() {
    printf("\nFork Test, pid %d\n", getpid());
    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp"
                     : "=r"(cur_sp));
        printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        ++cnt;
        if ((ret = fork()) != 0) {
            asm volatile("mov %0, sp"
                         : "=r"(cur_sp));
            printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        } else {
            while (cnt < 5) {
                asm volatile("mov %0, sp"
                             : "=r"(cur_sp));
                printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
                delay(1000000);
                ++cnt;
            }
        }
        exit(0);
    } else {
        volatile unsigned int __attribute__((aligned(16))) mbox[36];
        get_board_revision(mbox);
        mbox_call(MBOX_CH_PROP, mbox);
        for (int i = 0; i < 8; i++) {
            printf("mbox %d: %x\n", i, mbox[i]);
        }
        printf("parent here, pid %d, child %d\n", getpid(), ret);
    }
}

void cpu_timer_register_enable() {
    unsigned long tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
}

void main() {

    uart_init();
    init_printf(0, putc);
    buddy_system_init();
    mem_init();
    irq_vector_init();
    timer_init();
    sched_init();
    cpu_timer_register_enable();

    // thread_create(&foo);
    // thread_create(&foo);
    // fork_test();

    int ret;
    if ((ret = fork()) == 0)
        exec("syscall.img", 0x0);
    idle();
}
