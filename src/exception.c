#include "mini_uart.h"
#include "exception.h"
#include "string.h"
#include "timer.h"
#include "task.h"
#include "syscall.h"
#include "sched.h"
#include "signal.h"
void enable_interrupt(){
    asm volatile(
        "msr DAIFClr, 0xf"
    );
}

void disable_interrupt(){
    asm volatile(
        "msr DAIFSet, 0xf"
    );
}


void exception_entry(trap_frame* tf){
    // busy_wait_writes("EXCEPTION ENTRY",TRUE);
    // spsr_el1, elr_el1, and esr_el1
    unsigned long long reg_spsr_el1, reg_elr_el1,reg_esr_el1;
    asm volatile(
        "mrs %0,spsr_el1\n\t"
        "mrs %1,elr_el1\n\t"
        "mrs %2,esr_el1\n\t"
        : "=r" (reg_spsr_el1),
          "=r" (reg_elr_el1),
          "=r" (reg_esr_el1)
    );
    /*
    https://developer.arm.com/documentation/ddi0601/2021-12/AArch64-Registers/ESR-EL1--Exception-Syndrome-Register--EL1-?lang=en
    EC, bits [31:26], 0b010101: SVC instruction execution in AArch64 state.
    */
    unsigned long long ec = reg_esr_el1>>26;
    if(ec == 0b010101){
        switch (tf->x8)
        {
        case 0: // getpid
            sys_getpid(tf);
            break;
        case 1: // uart_read
            sys_uart_read(tf,(char*)tf->x0,tf->x1);
            break;
        case 2: // uart_write
            sys_uart_write(tf,(char*)tf->x0,tf->x1);
            break;
        case 3: // exec
            sys_exec(tf,(char*)tf->x0,(char**)tf->x1);
            break;
        case 4: // fork
            sys_fork(tf);
            break;
        case 5: // exit
            sys_exit(tf);
            break;
        case 6: // mbox_call
            sys_mbox_call(tf,(unsigned char)tf->x0,(unsigned int*)tf->x1);
            break;
        case 7: // kill
            sys_kill(tf,(int)tf->x0);
            break;
        case 8:
            signal_register(tf->x0, (void (*)())(tf->x1));
            break;
        case 9:
            signal_kill(tf->x0,tf->x1);
            break;
        case 10:
            sys_ls(tf);
            break;
        case 11:
            sys_open(tf,(const char*)(tf->x0),tf->x1);
            break;
        case 15:
            sys_mkdir(tf,(const char*)(tf->x0),0);
            break;
        case 21:
            sigreturn();
            break;
        default:
            break;
        }
    }
    else{
        disable_timer_interrupt();
        writes_uart("Exception\r\n");
        writes_uart("spsr_el1: ");
        writehex_uart(reg_spsr_el1,1);
        writes_uart("reg_elr_el1: ");
        writehex_uart(reg_elr_el1,1);
        writes_uart("reg_esr_el1: ");
        writehex_uart(reg_esr_el1,1);
        enable_timer_interrupt();
    }
    return;
}

int curr_task_privilege = 100;
void irq_entry(trap_frame* tf){
        
    // writes_uart("IRQ entry\r\n");
    // writes_uart("Interrupt Source:");
    // writehex_uart(*core0_interrupt_source,1);
    if(*core0_interrupt_source & (1<<8)){ //GPU interrupt
        //*AUX_MU_IER_REG = 0;
        // add_task(GPU_interrupt_handler,0);
        /* AUX_MU_IIR_REG bits 2:1 
        00 : No interrupts
        01 : Transmit holding register empty
        10 : Receiver holds valid byte
        11 : <Not possible> */
        
        if((*AUX_MU_IIR_REG & 0b0010)) //write interrupt
        {
            // disable_interrupt();
            *AUX_MU_IER_REG &= ~2; //disable write interrupt
            add_task(tx_interrupt_handler,INTERRUPT_PRIVILEGE_WRITE);
            if(curr_task_privilege<INTERRUPT_PRIVILEGE_WRITE)
                return;
            
            enable_interrupt();
            do_task(&curr_task_privilege);
            //*AUX_MU_IER_REG |= 2;
        }
        else if(*AUX_MU_IIR_REG & 0b0100) // read interrupt
        {
            // disable_interrupt();
            *AUX_MU_IER_REG &= ~1; // disable read interrupt
            add_task(rx_interrupt_handler,INTERRUPT_PRIVILEGE_READ);
            if(curr_task_privilege<INTERRUPT_PRIVILEGE_READ)
                return;
            enable_interrupt();
            do_task(&curr_task_privilege);
            
        }
        // asm volatile(
        //         "msr DAIFClr, 0xf"
        // );
        // do_task();
        //*AUX_MU_IER_REG = 3;
        // GPU_interrupt_handler();
    }  
    else if (*core0_interrupt_source & (0x00000002))
    {   
        //writes_nl_uart("TIMER INTERRUPT HANDLE");
        disable_timer_interrupt();
        
        add_task(Timer_interrupt_handler,INTERRUPT_PRIVILEGE_TIMER);
        if(curr_task_privilege<INTERRUPT_PRIVILEGE_TIMER)
                return;
        enable_interrupt();
        //writes_nl_uart("DO TIMER INTERRUPT NOW");
        do_task(&curr_task_privilege);
        // if(!is_timerq_empty())
        // {
        enable_timer_interrupt();
        schedule();
        //}
        // Timer_interrupt_handler();
    }
    else{
        writehex_uart(*core0_interrupt_source,1);
        writes_uart("Unknown irq \r\n");
    }

    // asm volatile(
    //     "msr DAIFClr, 0xf"
    // );
    // if the exception|irq is from user mode(el0), because kernel doesn't have content in Thread_struct.
    if ((tf->spsr_el1 & 0b1111) == 0)
    {
        run_signal(tf);
    }
    
    return;
}
void rx_interrupt_handler(){
    char r;
    r = (char)(*AUX_MU_IO_REG);
    r= (r=='\r')?'\n':r;
    uart_buf_read_push(r);
    *AUX_MU_IER_REG |= 1;
}
void tx_interrupt_handler(){
    if(!is_empty_write())
    {       
        *AUX_MU_IO_REG = uart_buf_write_pop();
        *AUX_MU_IER_REG |= 2;
    }else{
        *AUX_MU_IER_REG = 1;
    }
}

void Timer_interrupt_handler(){
    unsigned long long time_count=0;
    unsigned long long time_freq=0;
    asm volatile(
        "mrs %0,cntpct_el0\n\t"
        "mrs %1,cntfrq_el0\n\t"
        :"=r" (time_count),
         "=r" (time_freq)
        );
    //itr_timer_queue();
    // busy_wait_writes("TIMER INT",TRUE);
    // for schedule timer interrupt
    asm volatile(
        "msr cntp_tval_el0, %0\n\t"
        ::"r" (time_freq>>5)
    );
    // schedule();
    
    // if(is_timerq_empty()){
    //     // write_int_uart((int)(time_count/time_freq),0);
    //     // writes_uart(" seconds After booting\r\n");
    //     // set_expired_time(0x0fffffff);
    //     disable_timer_interrupt();
    //     return;
    // }
    // else{
    //     writes_uart("Current time is: ");
    //     write_int_uart((int)(time_count/time_freq),1);
    //     timer* h = get_head_timer();
    //     h->callback(h->message);
    //     // writes_uart("DEBUG:");
    //     // writes_nl_uart(h->message);
    //     if(h->next==nullptr){
    //         writes_uart("next timer is not exist.\r\n");
    //         h = to_next_timer();
    //         disable_timer_interrupt();
    //     }
    //     else{
    //         h = to_next_timer();
    //         writes_uart("Found next timer in ");
    //         write_int_uart(h->value,TRUE);
    //         unsigned long long time_count=0;
    //         unsigned long long time_freq=0;
    //         get_current_time(&time_count,&time_freq);
    //         set_expired_time(h->value - time_count/time_freq);
    //         enable_timer_interrupt();
    //     }
    // }
    
}