#include "mini_uart.h"
#include "exception.h"
#include "cpio.h"
#include "shell.h"
#include "stdlib.h"
#include "dtb.h"
#include "string.h"
#include "timer.h"
#include "task.h"
#include "allocator.h"
#include "sched.h"
#include "vfs.h"
extern unsigned long _head_start_brk;
extern Thread_struct* get_current();
void foo()
{
    for(int i = 0; i < 10; ++i) {
        unsigned long long reg_sp_el0;
        asm volatile(
            "mrs %0,sp_el0\n\t"
            : "=r" (reg_sp_el0)
        );
        writes_uart("Thread id: ");
        write_int_uart(get_current()->id,FALSE);
        writes_uart(" ");
        write_int_uart(i,TRUE);
        // delay(1000000);
        // unsigned long long time_count=0;
        // unsigned long long time_freq=0;
        // get_current_time(&time_count,&time_freq);
        // unsigned long long time_target = time_count + time_freq*0.01;
        // while(time_count < time_target){
        //     get_current_time(&time_count,&time_freq);
        // }
        schedule();
    }
    thread_exit();
    // get_current()->state = FINISHED;
}


int main(){
    // register unsigned long dtb_reg asm ("x15");
    
    //init_frame_freelist();
    uint64_t tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
    init_uart();
    init_uart_buf();
    init_taskq();
    init_memory();
    init_timer();
    init_sched();
    *AUX_MU_IER_REG = 1; // 3 for RX, TX interrupt enable
    *IRQ_ENABLE1 = 1<<29;
    // rootfs_init("tmpfs");
    

    writes_uart("\r\n");
    // writes_uart("██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗\r\n");
    // writes_uart("██║    ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝\r\n");
    // writes_uart("██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗  \r\n");
    // writes_uart("██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝  \r\n");
    // writes_uart("╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗\r\n");
    // writes_uart("╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝\r\n");
    
    writes_uart("Hello World!\r\n");
    // thread_create(testfs_exec);
    // idle();

    // for (int i = 0; i < 10; i++)
    // {
    //     thread_create(foo);
    // }
    // idle();
    // disable_timer_interrupt();
    while(1)
    { 
        // writes_uart("main\r\n");
        read_command();
        //read_uart_buf();
    }
    return 0;
}