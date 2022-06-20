#include "thread.h"

// extern void switch_to(struct thread_context *curr_context, struct thread_context *next_context, struct thread *next_thread);
extern void switch_to(unsigned long curr_context, unsigned long next_context, unsigned long next_thread, unsigned long pgd);
extern void switch_pgd(unsigned long pgd);
extern void from_EL1_to_EL0(unsigned long prog, unsigned long user_sp, unsigned long kernel_sp);
extern void store_context(struct thread_context *context);
extern void load_context(struct thread_context *context);
extern void sys_ret();
int a = 0;

struct thread* thread_dequeue(struct thread_queue* queue)
{
    struct thread* thread = NULL;
    
    if (queue->head != NULL) {
        thread = queue->head;

        if (queue->head->next == NULL) {
            queue->tail = NULL;
        }
        queue->head = queue->head->next;

        thread->next = NULL;
    }

    return thread;
}

void thread_enqueue(struct thread_queue* queue, struct thread* thread)
{
    if (queue->tail == NULL) {
        queue->head = thread;
    }
    else {
        queue->tail->next = thread;
    }

    queue->tail = thread;
    thread->next = NULL;
}

void thread_module_init()
{
    struct thread* kernel_thread;
    unsigned long *pgd;
    THREAD_START = 0;

    runnable_queue = mm_alloc(sizeof(struct thread_queue));
    runnable_queue->head = NULL;
    runnable_queue->tail = NULL;

    for (int i = 0; i < PID_NUM; i++) {
        pid_used[i] = false;
    }

    pgd = create_page_table();
    alloc_page_table(pgd, 0x6000, 0, BOOT_PGD_ATTR);
    kernel_thread             = mm_alloc(sizeof(struct thread));
    kernel_thread->pgd        = pgd;
    kernel_thread->pid        = get_new_pid();
    kernel_thread->parent     = NULL;
    kernel_thread->child_num  = 0;

    kernel_thread->code_addr  = (unsigned long) idle_thread;
    // kernel_thread->u_stack    = mm_alloc(THREAD_STACK_SIZE);
    // kernel_thread->k_stack    = mm_alloc(THREAD_STACK_SIZE);
    kernel_thread->k_stack    = (unsigned long)alloc_page_table(pgd, KERNEL_STACK_VA, 0, PD_RAM_ATTR);
    kernel_thread->u_stack    = (unsigned long)alloc_page_table(pgd, USER_STACK_VA  , 0, PD_USER_ATTR);
    kernel_thread->status     = THREAD_RUNNABLE;
    // kernel_thread->ksp        = (unsigned long) kernel_thread->k_stack + (THREAD_STACK_SIZE);
    // kernel_thread->privilege  = 1;

    kernel_thread->context.lr = (unsigned long) idle_thread;
    // kernel_thread->context.fp = (unsigned long) kernel_thread->u_stack + (THREAD_STACK_SIZE);
    // kernel_thread->context.sp = (unsigned long) kernel_thread->u_stack + (THREAD_STACK_SIZE);
    kernel_thread->context.fp = (unsigned long)USER_STACK_VA + THREAD_STACK_SIZE;
    kernel_thread->context.sp = (unsigned long)USER_STACK_VA + THREAD_STACK_SIZE;
    kernel_thread->next       = NULL;

    for (int i = 0; i < THREAD_MAX_SIG_NUM; i++) {
        kernel_thread->sig_handlers[i] = NULL;
        kernel_thread->sig_num[i] = 0;
    }
    kernel_thread->under_sig  = 0;

    for (int i = PERIPHERAL_BASE; i < PERIPHERAL_END; i = i + PAGE_TABLE_SIZE) {
        alloc_page_table(pgd, i, i, PD_USER_ATTR);
    }

    // sync_uart_puts("kernel_thread->k_stack: 0x");
    // uart_hex((unsigned long) kernel_thread->k_stack);
    // sync_uart_puts("\nkernel_thread->u_stack: 0x");
    // uart_hex((unsigned long) kernel_thread->u_stack);
    // sync_uart_puts("\n\n");

    thread_enqueue(runnable_queue, kernel_thread);
    asm volatile ("msr tpidr_el1, %0" : : "r"(kernel_thread));
}

pid_t get_new_pid()
{
    for (int i = 0; i < PID_NUM; i++) {
        if (pid_used[i] == false) {
            pid_used[i] = true;
            return i;
        }
    }

    return -1;
}

struct thread* thread_create(void (*func) ())
{
    struct thread *new_thread; // thread descriptor
    unsigned long *pgd;
    
    pgd = create_page_table();
    alloc_page_table(pgd, 0x6000, 0, BOOT_PGD_ATTR);
    new_thread             = mm_alloc(sizeof(struct thread));
    new_thread->pgd        = pgd;
    new_thread->pid        = get_new_pid();
    new_thread->parent     = NULL;
    new_thread->child_num  = 0;

    new_thread->code_addr  = func;
    // new_thread->u_stack    = mm_alloc(THREAD_STACK_SIZE);
    // new_thread->k_stack    = mm_alloc(THREAD_STACK_SIZE);
    new_thread->k_stack    = (unsigned long)alloc_page_table(pgd, KERNEL_STACK_VA, 0, PD_RAM_ATTR);
    new_thread->u_stack    = (unsigned long)alloc_page_table(pgd, USER_STACK_VA  , 0, PD_USER_ATTR);
    new_thread->status     = THREAD_RUNNABLE;
    // new_thread->ksp        = (unsigned long) new_thread->k_stack + (THREAD_STACK_SIZE);
    // new_thread->privilege  = 0;

    new_thread->context.lr = (unsigned long) func;
    new_thread->context.fp = (unsigned long)USER_STACK_VA + THREAD_STACK_SIZE;
    new_thread->context.sp = (unsigned long)USER_STACK_VA + THREAD_STACK_SIZE;
    new_thread->next       = NULL;

    for (int i = 0; i < THREAD_MAX_SIG_NUM; i++) {
        new_thread->sig_handlers[i] = NULL;
        new_thread->sig_num[i] = 0;
    }
    new_thread->under_sig = 0;

    for (int i = PERIPHERAL_BASE; i < PERIPHERAL_END; i = i + PAGE_TABLE_SIZE){
        alloc_page_table(pgd, i, i, PD_USER_ATTR);
    }

    thread_enqueue(runnable_queue, new_thread);

    //sync_uart_puts("----------------- Thread_create --------------\n");
    // sync_uart_puts("New thread is created at 0x");
    // uart_hex(new_thread);
    // sync_uart_puts(" with pid is ");
    // uart_dec(new_thread->pid);
    // sync_uart_puts("\n");

    // sync_uart_puts("LR at 0x");
    // uart_hex(new_thread->context.lr);
    // sync_uart_puts("\nSP at 0x");
    // uart_hex(new_thread->context.sp);
    // sync_uart_puts("\nKSP at 0x");
    // uart_hex(new_thread->ksp);
    // sync_uart_puts("\n\n");

    return new_thread;
}

struct thread* get_cur_thread()
{
    struct thread* cur_thread;

    asm volatile ("mrs %0, tpidr_el1" : "=r"(cur_thread));

    return cur_thread;
}

pid_t get_cur_thread_id()
{
    struct thread* cur_thread;
    pid_t pid;

    //sync_uart_puts("---------------- Get_cur_thread_id -----------\n");

    asm volatile ("mrs %0, tpidr_el1" : "=r"(cur_thread));
    pid = cur_thread->pid;

    // sync_uart_puts("pid = ");
    // uart_dec(pid);
    // sync_uart_puts("\n");

    return pid;
}

void thread_schedule()
{
    struct thread *cur_thread, *nxt_thread;

    cur_thread = get_cur_thread();
    // sync_uart_puts("----------------Thread schedule-----------\n");
    check_signal();

    while (1) {

        nxt_thread = thread_dequeue(runnable_queue);

        if (nxt_thread == NULL) {
            //sync_uart_puts("nxt_thread == NULL\n");
            break;
        }
        else if (nxt_thread->status == THREAD_RUNNABLE) {
            //sync_uart_puts("nxt_thread->status == THREAD_RUNNABLE\n");
            break;
        }
        else if (nxt_thread->status == THREAD_EXIT) {
            if (nxt_thread->child_num == 0) {
                if (nxt_thread->parent != NULL) {
                    nxt_thread->parent->child_num -= 1;
                }
                //sync_uart_puts("nxt_thread->status == THREAD_EXIT\n");
                pid_used[nxt_thread->pid] = false;
                buddy_free((void *)nxt_thread->k_stack);
                buddy_free((void *)nxt_thread->u_stack);
                mm_free((void *)nxt_thread);
            }
        }
    }

    if (nxt_thread != NULL) {
        
        // sync_uart_puts("cur_thread: 0x:");
        // uart_hex((unsigned long) cur_thread);
        // sync_uart_puts(", nxt_thread: 0x");
        // uart_hex((unsigned long) nxt_thread);
        // sync_uart_puts("\n");
        // if (a==1 && cur_thread->pid == 0)
        //     sync_uart_puts("hi");
        // sync_uart_puts("cur_thread id: ");
        // uart_dec((unsigned long) cur_thread->pid);
        // sync_uart_puts(", next_thread id: ");
        // uart_dec((unsigned long) nxt_thread->pid);
        // sync_uart_puts("\n");

        // sync_uart_puts("cur_thread privilege: ");
        // uart_dec((unsigned long) cur_thread->privilege);
        // sync_uart_puts(", next_thread privilege: ");
        // uart_dec((unsigned long) nxt_thread->privilege);
        // sync_uart_puts("\n");

        // unsigned long ksp, usp;
        // asm volatile("mov %0, sp" : "=r"(ksp));
        // asm volatile("mrs %0, sp_el0" : "=r"(usp));
        // sync_uart_puts("ksp: 0x");
        // uart_hex((unsigned long) ksp);
        // sync_uart_puts("\n");
        // sync_uart_puts("usp: 0x");
        // uart_hex((unsigned long) usp);
        // sync_uart_puts("\n");
        
        thread_enqueue(runnable_queue, cur_thread);

        // unsigned long current_el;
        // asm volatile ("mrs %0, CurrentEL" : "=r" (current_el));
        // current_el = current_el >> 2;
        // sync_uart_puts("Current EL: 0x");
        // uart_hex(current_el);
        // sync_uart_puts("\n");
        //sync_uart_puts("---------------- Switch to -----------\n");
        if (cur_thread == nxt_thread)
            return;
        // if (cur_thread->pid == 0 && nxt_thread->pid == 2)
        //     ;
        
        //switch_to(&cur_thread->context, &nxt_thread->context, nxt_thread);
        switch_to(&cur_thread->context, &nxt_thread->context, nxt_thread, va_to_pa((unsigned long)(nxt_thread->pgd)));
    }
}

void idle_thread()
{
    while (1) {
        //sync_uart_puts("This is idle thread.\n");
        int i = 10;
        while(i-- > 0) asm volatile ("nop");
        thread_schedule();
    }
}

void thread_exit()
{
    struct thread *cur_thread;

    cur_thread = get_cur_thread();
    if (cur_thread->pid != 0)
        cur_thread->status = THREAD_EXIT;
    // sync_uart_puts("-----Thread_exit.----\n");
    // sync_uart_puts("cur_thread id: ");
    // uart_dec(cur_thread->pid);
    // sync_uart_puts("\n");
    // sync_uart_puts("cur_thread child num: ");
    // uart_dec(cur_thread->child_num);
    // sync_uart_puts("\n");
    // sync_uart_puts("cur_thread parent: 0x");
    // uart_hex(cur_thread->parent);
    // sync_uart_puts("\n");
    // sync_uart_puts("---------------------\n");

    thread_schedule();
}

void thread_exec (void (*prog)(), size_t prog_size) {

    struct thread *new_thread = NULL;
    pid_t new_pid;
    unsigned long *pgd;
    unsigned long current_el;

    new_pid = get_new_pid();
    //sync_uart_puts("---------------- Thread_exec ------------------\n");
    //uart_sdec("thread id = ", new_pid, "\n");

    if (new_pid != -1)
    {
        pgd = create_page_table();
        alloc_page_table(pgd, 0x6000, 0, BOOT_PGD_ATTR);

        for (int i = 0; i < prog_size; i += PAGE_TABLE_SIZE) {
            unsigned long va = USER_PROG_VA + i;
            unsigned long pa = va_to_pa(prog + i);

            alloc_page_table(pgd, va, pa, PD_USER_ATTR);
        }

        for (int i = PERIPHERAL_BASE; i < PERIPHERAL_END; i = i + PAGE_TABLE_SIZE) {
            alloc_page_table(pgd, i, i, PD_USER_ATTR);
        }

        new_thread             = mm_alloc(sizeof(struct thread));
        //uart_sdec("thread at = 0x", new_thread, "\n");
        new_thread->pgd        = pgd;
        new_thread->pid        = new_pid;
        new_thread->parent     = NULL;
        new_thread->child_num  = 0;

        // new_thread->code_addr  = (unsigned long)prog;
        // new_thread->u_stack    = mm_alloc(THREAD_STACK_SIZE);
        // new_thread->k_stack    = mm_alloc(THREAD_STACK_SIZE);
        new_thread->status     = THREAD_RUNNABLE;
        // new_thread->ksp        = (unsigned long) new_thread->k_stack + (THREAD_STACK_SIZE);
        // new_thread->privilege  = 0;

        new_thread->code_addr  = (unsigned long)USER_PROG_VA;
        new_thread->k_stack    = (unsigned long)alloc_page_table(pgd, KERNEL_STACK_VA, 0, PD_RAM_ATTR);
        new_thread->u_stack    = (unsigned long)alloc_page_table(pgd, USER_STACK_VA  , 0, PD_USER_ATTR);
        new_thread->code_size    = prog_size;

        new_thread->context.lr = (unsigned long)USER_PROG_VA;
        new_thread->context.fp = (unsigned long)USER_STACK_VA + THREAD_STACK_SIZE;
        new_thread->context.sp = (unsigned long)USER_STACK_VA + THREAD_STACK_SIZE;

        // new_thread->context.lr = (unsigned long) prog;
        // new_thread->context.fp = (unsigned long) new_thread->u_stack + (THREAD_STACK_SIZE);
        // new_thread->context.sp = (unsigned long) new_thread->u_stack + (THREAD_STACK_SIZE);
        new_thread->next       = NULL;

        for (int i = 0; i < THREAD_MAX_SIG_NUM; i++) {
            new_thread->sig_handlers[i] = NULL;
            new_thread->sig_num[i] = 0;
        }
        new_thread->under_sig = 0;

        // Get current EL
        asm volatile ("mrs %0, CurrentEL" : "=r" (current_el));
        current_el = current_el >> 2;

        // Print prompt
        sync_uart_puts("Current EL: 0x");
        uart_hex(current_el);
        sync_uart_puts("\n");

        sync_uart_puts("User program pid: ");
        uart_dec((unsigned long) new_thread->pid);
        sync_uart_puts("\n");

        sync_uart_puts("User program at: 0x");
        uart_hex((unsigned long) prog);
        sync_uart_puts("\n");
        sync_uart_puts("User program stack top: 0x");
        uart_hex((unsigned long) new_thread->context.sp);
        sync_uart_puts("\n");
        sync_uart_puts("Kernel stack top: 0x");
        uart_hex((unsigned long)new_thread->k_stack + (THREAD_STACK_SIZE));
        sync_uart_puts("\n");
        sync_uart_puts("-----------------Entering user program-----------------\n");

        asm volatile ("msr tpidr_el1, %0" : : "r"(new_thread));

        unsigned long tmp;
        asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
        tmp |= 1;
        asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
        set_timer_interrupt(true);
        THREAD_START = 1;
        // from_EL1_to_EL0((unsigned long)prog, (unsigned long)new_thread->u_stack + (THREAD_STACK_SIZE), (unsigned long)new_thread->k_stack + (THREAD_STACK_SIZE));
        switch_pgd(va_to_pa((unsigned long)new_thread->pgd));
        from_EL1_to_EL0((unsigned long)USER_PROG_VA, (unsigned long)USER_STACK_VA + THREAD_STACK_SIZE, (unsigned long)KERNEL_STACK_VA + THREAD_STACK_SIZE);
    }

    sync_uart_puts("exec end!\n");
    
    // asm volatile ("mrs %0, CurrentEL" : "=r" (current_el));
    // current_el = current_el >> 2;
    // sync_uart_puts("Current EL: 0x");
    // uart_hex(current_el);
    // sync_uart_puts("\n");
}

pid_t thread_fork (struct trap_frame* trap_frame) {
    
    struct thread *parent_thread;
    struct thread *child_thread;
    struct thread *cur_thread;
    //struct trap_frame *child_trap_frame;
    unsigned long prog_base;

    unsigned long k_offset, u_offset, trap_offset;
    //unsigned long ksp, usp;
    // sync_uart_puts("--------------- Thread_fork ----------------\n");
    parent_thread = get_cur_thread();
    prog_base     = buddy_alloc(log2_ceiling(parent_thread->code_size / PAGE_SIZE));
    // child_thread  = thread_create((void *)parent_thread->code_addr);
    child_thread  = thread_create(prog_base);

    //asm volatile("mov %0, sp" : "=r"(ksp));
    //asm volatile("mrs %0, sp_el0" : "=r"(usp));

    // k_offset = (unsigned long) parent_thread->k_stack + THREAD_STACK_SIZE - ksp;
    // u_offset = (unsigned long) parent_thread->u_stack + THREAD_STACK_SIZE - usp;
    // trap_offset = (unsigned long) parent_thread->k_stack + THREAD_STACK_SIZE - (unsigned long) trap_frame;
    // sync_uart_puts("k_offset: 0x");
    // uart_hex((unsigned long)k_offset);
    // sync_uart_puts("\n");
    // sync_uart_puts("u_offset: 0x");
    // uart_hex((unsigned long)u_offset);
    // sync_uart_puts("\n");
    // sync_uart_puts("ksp: 0x");
    // uart_hex((unsigned long)ksp);
    // sync_uart_puts("\n");
    // sync_uart_puts("usp: 0x");
    // uart_hex((unsigned long)usp);
    // sync_uart_puts("\n");
    //child_trap_frame = (struct trap_frame *)((unsigned long) child_thread->k_stack + THREAD_STACK_SIZE - trap_offset);

    store_context(&child_thread->context);
    cur_thread = get_cur_thread();

    if (cur_thread->pid == parent_thread->pid) {
        
        child_thread->code_size = parent_thread->code_size;

        for (int i = 0; i < child_thread->code_size; i++)  {
            ((char *)prog_base)[i] = ((char *)(parent_thread->code_addr))[i];
        }

        for (int i = 0; i < child_thread->code_size; i += PAGE_TABLE_SIZE) {
            unsigned long va = USER_PROG_VA + i;
            unsigned long pa = va_to_pa(child_thread->code_addr + i);

            alloc_page_table(child_thread->pgd, va, pa, PD_USER_ATTR);
        }
        // trap_frame->regs[0] = child_thread->pid;
        //sync_uart_puts("44\n");
        for (int i = 0; i < THREAD_STACK_SIZE; i++) {
            ((char *) child_thread->k_stack)[i] = ((char *) parent_thread->k_stack)[i];
            ((char *) child_thread->u_stack)[i] = ((char *) parent_thread->u_stack)[i];
        }

        for (int i = 0; i < THREAD_MAX_SIG_NUM; i++) {
            child_thread->sig_handlers[i] = parent_thread->sig_handlers[i];
            child_thread->sig_num[i] = parent_thread->sig_num[i];
        }
        child_thread->under_sig = parent_thread->under_sig;

        // sync_uart_puts("Without offset child_thread->context.sp: 0x");
        // uart_hex((unsigned long) child_thread->context.sp);
        // sync_uart_puts(" child_thread->context.fp: 0x");
        // uart_hex((unsigned long) child_thread->context.fp);
        // sync_uart_puts("\n");

        // sync_uart_puts("Without offset child_trap_frame->sp_el0: 0x");
        // uart_hex((unsigned long)child_trap_frame->sp_el0);
        // sync_uart_puts("\n");

        // sync_uart_puts("child_trap_frame: 0x");
        // uart_hex((unsigned long)child_trap_frame);
        // sync_uart_puts("\n");

        //child_thread->context.sp = (unsigned long) child_thread->k_stack + THREAD_STACK_SIZE - k_offset;
        
        //child_thread->context.fp = (unsigned long) child_thread->k_stack + THREAD_STACK_SIZE - ((unsigned long) parent_thread->k_stack + THREAD_STACK_SIZE - child_thread->context.fp);
        //sync_uart_puts("10\n");
        //child_trap_frame->sp_el0 = (unsigned long) child_thread->u_stack + THREAD_STACK_SIZE - u_offset;
        //sync_uart_puts("11\n");
        //child_trap_frame->regs[0] = 0;
        
        child_thread->parent = parent_thread;
        parent_thread->child_num +=1;

        // sync_uart_puts("child_trap_frame->sp_el0: 0x");
        // uart_hex((unsigned long)child_trap_frame->sp_el0);
        // sync_uart_puts("\n");

        // sync_uart_puts("trap_frame->sp_el0: 0x");
        // uart_hex((unsigned long)trap_frame->sp_el0);
        // sync_uart_puts("\n");

        // sync_uart_puts("parent_thread id: ");
        // uart_dec((unsigned long) parent_thread->pid);
        // sync_uart_puts(", child_thread id: ");
        // uart_dec((unsigned long) child_thread->pid);
        // sync_uart_puts("\n");

        // sync_uart_puts("child->k_stack: 0x");
        // uart_hex((unsigned long)child_thread->k_stack);
        // sync_uart_puts("\nchild->u_stack: 0x");
        // uart_hex((unsigned long)child_thread->u_stack);
        // sync_uart_puts("\n");

        // sync_uart_puts("parent->k_stack: 0x");
        // uart_hex((unsigned long)parent_thread->k_stack);
        // sync_uart_puts("\nparent->u_stack: 0x");
        // uart_hex((unsigned long)parent_thread->u_stack);
        // sync_uart_puts("\n");

        // sync_uart_puts("parent trap_frame: 0x");
        // uart_hex((unsigned long) trap_frame);
        // sync_uart_puts(" child_trap_frame: 0x");
        // uart_hex((unsigned long) child_trap_frame);
        // sync_uart_puts("\n");

        // sync_uart_puts("child_thread->context.sp: 0x");
        // uart_hex((unsigned long) child_thread->context.sp);
        // sync_uart_puts(" child_thread->context.fp: 0x");
        // uart_hex((unsigned long) child_thread->context.fp);
        // sync_uart_puts("\n");

        // sync_uart_puts("cur_thread id: ");
        // uart_dec((unsigned long) runnable_queue->head->pid);
        // sync_uart_puts(", tail_thread id: ");
        // uart_dec((unsigned long) runnable_queue->tail->pid);
        // sync_uart_puts("\n");
        // sync_uart_puts("--------------- Thread_fork parent end ----------------\nreturn pid: ");
        // uart_dec(child_thread->pid);
        // sync_uart_puts("\n");
        return child_thread->pid;
        // thread_schedule();
    }
    // sync_uart_puts("--------------- Thread_fork child end ----------------\n");
    return 0;
}

void thread_kill(pid_t pid)
{
    struct thread* cur_thread;

    cur_thread = runnable_queue->head;
    sync_uart_puts("------------ Thread_kill -------------\n");

    if (pid == 0) return;

    while(cur_thread != NULL) {
        if (cur_thread->pid == pid) {
            cur_thread->status = THREAD_EXIT;
            sync_uart_puts("Thread id: ");
            uart_dec((unsigned long) cur_thread->pid);
            sync_uart_puts(" has been killed.\n\n");
            
            break;
        }
        cur_thread = cur_thread->next;
    }

    if (cur_thread == NULL) {
        sync_uart_puts("Thread id: ");
        uart_dec((unsigned long) pid);
        sync_uart_puts(" not found.\n\n");
    }
}

void default_sig_handler()
{
    thread_exit();
}

void check_signal() {

    void (*handler) ();
    void *sig_ustack;
    struct thread *cur_thread = get_cur_thread();

    unsigned long sp, spsr_el1;
    //sync_uart_puts("hi\n");
    //uart_sdec("cur_thread id: ", cur_thread->pid, "\n");
    while (1) {
        //uart_sdec("cur_thread->under_sig: ", cur_thread->under_sig, "\n");
        if (cur_thread->under_sig == 1)
            break;
        store_context(&cur_thread->sig_context);
        // if (a == 1) {
        //     asm volatile("mov %0, sp" : "=r"(sp));
        //     sp += 8;
        //     sync_uart_puts("LR: 0x");
        //     uart_hex(*((unsigned long*) sp));
        //     sync_uart_puts("\n");
        //     sync_uart_puts("SP: 0x");
        //     uart_hex(sp);
        //     sync_uart_puts("\n");
        // }
        handler = get_sig_handler(cur_thread);
        // uart_shex("handler: 0x", handler, "\n");

        if (handler == NULL) {
            // if (a==1)
            //     sync_uart_puts("a==1handler!!\n");
            break;
        }
        else {
            
            if (handler == default_sig_handler) {
                // Run in kernel mode
                // cur_thread->under_sig = 1;
                handler();
            }
            else {
                cur_thread->under_sig = 1;
                // a = 1;
                // sync_uart_puts("handler!!\n");
                // sig_ustack = buddy_alloc(2);
                sig_ustack    = (unsigned long)alloc_page_table(cur_thread->pgd, USER_STACK_VA - THREAD_STACK_SIZE, 0, PD_USER_ATTR);
                // asm volatile("mov %0, sp" : "=r"(sp));
                // sp += 8;
                // sync_uart_puts("LR: 0x");
                // uart_hex(*((unsigned long*) sp));
                // sync_uart_puts("\n");
                // sync_uart_puts("SP: 0x");
                // uart_hex(sp);
                // sync_uart_puts("\n");

                // Run in user mode
                asm volatile("msr     elr_el1, %0 \n\t"
                             "msr     sp_el0,  %1 \n\t"
                             "mov     lr, %2       \n\t"
                             "eret" :: "r" (handler), "r"((unsigned long)USER_STACK_VA ), "r" (sys_ret));

                // mm_free(sig_ustack);
            }
        }

    }
    // if (a == 1) {
    //     asm volatile("mov %0, sp" : "=r"(sp));
    //     sp += 8;
    //     sync_uart_puts("LR: 0x");
    //     uart_hex(*((unsigned long*) sp));
    //     sync_uart_puts("\n");
    //     sync_uart_puts("SP: 0x");
    //     uart_hex(sp);
    //     sync_uart_puts("\n");
    // }
}

void *get_sig_handler (struct thread* thread) {

    void *handler = NULL;

    for (int i = 0; i < THREAD_MAX_SIG_NUM; i++)
    {

        if (thread->sig_num[i] > 0)
        {

            if (thread->sig_handlers[i] == NULL) {
                // Default signal handler
                handler = default_sig_handler;
            }
            else {
                handler = thread->sig_handlers[i];
            }

            thread->sig_num[i] -= 1;

            break;
        }

    }

    return handler;
}

void thread_sig_register (int sig, void (*handler)()) {
    
    struct thread* cur_thread;

    cur_thread = get_cur_thread();

    uart_shex("handler at: 0x", handler, "\n");

    cur_thread->sig_handlers[sig] = handler;

}

void thread_sig_kill (int pid, int sig) {

    struct thread* thread;

    thread = runnable_queue->head;

    while (thread != NULL)
    {
        if (thread->pid == pid) {
            thread->sig_num[sig] += 1;
            break;
        }

        thread = thread->next;
    }

    return;
}

void thread_sig_return () {

    struct thread* cur_thread;

    cur_thread = get_cur_thread();
    cur_thread->under_sig = 0;
    // sync_uart_puts("sig return pid: ");
    // uart_dec(cur_thread->pid);
    // sync_uart_puts("\n");
    a = 1;
    // sync_uart_puts("thread_sig_return SP: 0x");
    // uart_hex(cur_thread->sig_context.sp);
    // sync_uart_puts("\n");
    load_context(&cur_thread->sig_context);

}