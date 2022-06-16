#include "exception.h"

void svc_handler(trap_frame_t* trap_frame)
{
    unsigned int svc = trap_frame->regs[8];
    unsigned long ksp, usp;
    unsigned long SPSR;
    

    // asm volatile("mrs %0, spsr_el1" : "=r"(SPSR));

    // sync_uart_puts("SPSR = 0x");
    // uart_hex(SPSR);
    // sync_uart_puts("\n");

    //sync_uart_puts("----------------- svc_handler --------------\n");
    // sync_uart_puts("Trap_frame is at 0x");
    // uart_hex(trap_frame);
    // sync_uart_puts(" with svc is ");
    // uart_dec(svc);
    // sync_uart_puts("\n\n");
    // sync_uart_puts("svc handler pid = ");
    // uart_dec(get_cur_thread_id());
    // sync_uart_puts(" svc = ");
    // uart_dec(svc);
    // sync_uart_puts("\n");
    switch (svc) {
        case 0:
            // asm volatile("mov %0, sp" : "=r"(ksp));
            // asm volatile("mrs %0, sp_el0" : "=r"(usp));
            // sync_uart_puts("ksp: 0x");
            // uart_hex((unsigned long)ksp);
            // sync_uart_puts("\n");
            // sync_uart_puts("usp: 0x");
            // uart_hex((unsigned long)usp);
            // sync_uart_puts("\n");
            trap_frame->regs[0] = get_cur_thread_id();
            // sync_uart_puts("\ntrap_frame->elr_el1 = 0x");
            // uart_hex(trap_frame->elr_el1);
            // sync_uart_puts("\n");
            break;
        case 1:
            set_interrupt(true);
            trap_frame->regs[0] = uartread((char *) trap_frame->regs[0], (size_t) trap_frame->regs[1]);
            break;
        case 2:
            
            // asm volatile("mov %0, sp" : "=r"(ksp));
            // asm volatile("mrs %0, sp_el0" : "=r"(usp));
            // sync_uart_puts("ksp: 0x");
            // uart_hex((unsigned long)ksp);
            // sync_uart_puts("\n");
            // sync_uart_puts("usp: 0x");
            // uart_hex((unsigned long)usp);
            // sync_uart_puts("\n");

            // sync_uart_puts("\n(char *) trap_frame->regs[0]: 0x");
            // uart_hex(trap_frame->regs[0]);
            // sync_uart_puts("\n");
            // sync_uart_puts("\nsize = ");
            // uart_dec(trap_frame->regs[1]);
            // sync_uart_puts("\n");
            // sync_uart_puts("\nvalue = ");
            // uart_send(*((char *) trap_frame->regs[0]));
            // sync_uart_puts("\n");
            trap_frame->regs[0] = uartwrite((char *) trap_frame->regs[0], (size_t) trap_frame->regs[1]);
            break;
        case 3:
            trap_frame->regs[0] = exec(trap_frame->regs[0], trap_frame->regs[1], trap_frame);
            break;
        case 4:
            thread_fork(trap_frame);
            
            // sync_uart_puts("&trap_frame->regs[0] = 0x");
            // uart_hex(&trap_frame->regs[0]);
            // sync_uart_puts("\n");
            // sync_uart_puts("trap_frame = 0x");
            // uart_hex(trap_frame);
            // sync_uart_puts("\n");
            // asm volatile("mov %0, sp" : "=r"(ksp));
            // sync_uart_puts("ksp: 0x");
            // uart_hex((unsigned long) ksp);
            // sync_uart_puts("\n");
            break;
        case 5:
            thread_exit();
            break;
        case 6:
            trap_frame->regs[0] = sys_mbox_call((unsigned char) trap_frame->regs[0], (unsigned int *) trap_frame->regs[1]);
            // sync_uart_puts("trap_frame->elr_el1 = 0x");
            // uart_hex(trap_frame->elr_el1);
            // sync_uart_puts("\n");
            // sync_uart_puts("mbox call end\npid = ");
            // uart_dec(get_cur_thread_id());
            // sync_uart_puts("\n\n");
            break;
        case 7:
            thread_kill(trap_frame->regs[0]);
            break;
        case 8:
            thread_sig_register(trap_frame->regs[0], trap_frame->regs[1]);
            break;
        case 9:
            thread_sig_kill(trap_frame->regs[0], trap_frame->regs[1]);
            break;
        case 10:
            thread_sig_return();
            break;
        default:
            sync_uart_puts("svc = ");
            uart_dec(svc);
            sync_uart_puts(" is not supported.\n");
            break;
    }
    
}

int exec(const char *name, char *const argv[], trap_frame_t* trap_frame)
{
    cpio_fp_t fp;
    struct thread* cur_thread;
    size_t prog_size;
    char *prog_addr;

    cpio_get_file_info(name, &fp);
    prog_size = ascii2int(fp.header->c_filesize, 8);
    prog_addr = mm_alloc(prog_size);
    prog_addr = (unsigned long) prog_addr - 0x10;
    copy_prog_from_cpio(prog_addr, fp.data, prog_size);
    
    cur_thread = get_cur_thread();

    cur_thread->code_addr  = (unsigned long) prog_addr;
    cur_thread->context.lr = (unsigned long) prog_addr;
    cur_thread->context.fp = (unsigned long) cur_thread->u_stack + (THREAD_STACK_SIZE);
    cur_thread->context.sp = (unsigned long) cur_thread->u_stack + (THREAD_STACK_SIZE);

    // trap_frame->spsr_el1;
    trap_frame->elr_el1 = (unsigned long) prog_addr;
    trap_frame->sp_el0 = (unsigned long) cur_thread->u_stack + (THREAD_STACK_SIZE);

    return 1;
}

unsigned int interrupt_lock = 0;

void set_interrupt(int enable) {

    if (enable)
    {
        if (interrupt_lock > 0)
        {
            interrupt_lock--;
        }
        
        if (interrupt_lock == 0)
        {
            asm volatile ("msr DAIFClr, 0xf");
        }
        
    }
    else
    {
        interrupt_lock++;
        asm volatile ("msr DAIFSet, 0xf");
    }

    return;
}
