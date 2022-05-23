#include "thread.h"

#include "uart.h"
#include "utils.h"
#include "gpio.h"
#include "cpio.h"
#include "command.h"
#include "mem.h"
#include "mmu.h"
#include "mbox.h"

Thread_List thread_list;

void schedule(){
	disable_current_interrupt();

    if(thread_list.beg == thread_list.end) {
		// no other task, all done
	}
	else {
		do {
			thread_list.end->next = thread_list.beg;
			thread_list.end = thread_list.beg;
			thread_list.beg = thread_list.beg->next;
			thread_list.end->next = NULLPTR;
		} while(thread_list.beg->iszombie == 1);// skip zombie
        load_pgd(thread_list.beg->pgd);
        switch_to(get_current(), &thread_list.beg->context);
	}

	enable_current_interrupt();
}

void kill_zombies(){
    disable_current_interrupt();

    do {
        if (thread_list.beg->iszombie) {
            Thread *zombie = thread_list.beg;
            thread_list.beg = thread_list.beg->next;
            zombie->iszombie = 0;
            kfree(zombie->user_stack);
            kfree(zombie->kernel_stack);
            page_free(zombie->pgd, VIRTUAL_KERNEL_STACK);
            kfree(zombie);
        }
        else {
            thread_list.end->next = thread_list.beg;
            thread_list.end = thread_list.beg;
            thread_list.beg = thread_list.beg->next;
            thread_list.end->next = NULLPTR;
        }
    } while(thread_list.beg->pid);// idle thread

    enable_current_interrupt();
}

void idle() {
    while (1) {
	    // printf("idle");
        kill_zombies(); // reclaim threads marked as DEAD
        schedule(); // switch to any other runnable thread
	}
}

Thread *thread_create(void *program_start) {
	// disable_current_interrupt();

    Thread *new = kmalloc(THREAD_SIZE);

    unsigned long *pgd = create_page_table();
    new->pgd = (unsigned long)pgd - KVA;

    new->pid = thread_list.pid_cnt++;
    new->parent = new->iszombie = 0;

    page_table_alloc((unsigned long)pgd, 0x6000, BOOT_PGD_ATTR, 0);
    new->user_stack = (char*)page_alloc((unsigned long)pgd, VIRTUAL_USER_STACK, 0, USER_READ_WRITE);
    new->kernel_stack = (char*)page_alloc((unsigned long)pgd, VIRTUAL_KERNEL_STACK, 0, PD_RAM_ATTR);

	new->context.fp = VIRTUAL_USER_STACK + USER_STACK_SIZE;
    new->context.lr = (unsigned long)program_start;
    new->context.sp = VIRTUAL_USER_STACK + USER_STACK_SIZE;

	new->next = NULLPTR;
	thread_list.end->next = new;
	thread_list.end = new;

	// enable_current_interrupt();
    
	return new;
}

void init_schedule() {
    timer_register();
    user_default_paging();
    thread_list.beg = thread_list.end = NULLPTR;
    Thread* init = thread_create(idle);
	asm volatile("msr tpidr_el1, %0\n"::"r"((unsigned long)init));

	thread_list.beg = thread_list.end = init;

    // set_time_shift(5);
    // enable_timer_interrupt();

    // run();
    // load_cpio("syscall.img");

    // for(int i = 0; i < 3; ++i) { // N should > 2
    //     thread_create(foo);
    // }

}

void exec_thread(char *data, unsigned int filesize) {
	disable_current_interrupt();

    Thread *user_thread = thread_create(data);
	user_thread->next = thread_list.beg;
	thread_list.beg = user_thread;
    thread_list.end = thread_list.beg->next;
    thread_list.end->next = NULLPTR;

    user_thread->program_size = filesize;
	user_thread->context.lr = VIRTUAL_USER_PROGRAM;

    // char *a = kmalloc(4096);
    // char *b = kmalloc(400);
    // char *c = kmalloc(4000);
    // kfree(a);
    // kfree(b);
    int tmpsize = 0;
    while (tmpsize < filesize) {
        int tmprange = (filesize-tmpsize) > 4096 ? 4096 : (filesize-tmpsize);
        // printf("-------------\n");
        user_thread->program = (char*)page_alloc((unsigned long)user_thread->pgd, VIRTUAL_USER_PROGRAM + tmpsize , 0, USER_READ_WRITE);
        for (int i = 0; i < tmprange; i++) {
            user_thread->program[i] = data[tmpsize+i];
        }
        tmpsize += 4096;
    }
    // kfree(c);

    load_pgd(user_thread->pgd);
    enable_current_interrupt();
    asm volatile("msr tpidr_el1, %0	\n": :"r"(&user_thread->context));
    asm volatile("msr elr_el1, %0	\n": :"r"(user_thread->context.lr));
    asm volatile("msr spsr_el1, xzr	\n");
    asm volatile("msr sp_el0, %0	\n": :"r"(user_thread->context.sp));
    asm volatile("mov sp, %0	    \n": :"r"(VIRTUAL_KERNEL_STACK + KERNEL_STACK_SIZE));
    asm volatile("eret	\n");

	// enable_current_interrupt();
}

void jump_thread(char *data, unsigned int filesize) {
	
    Thread *user_thread = thread_list.beg;

    // user_thread->program = (char *)(PHYSICAL_USER_PROGRAM+KVA);
    user_thread->program_size = filesize;

	user_thread->context.lr = VIRTUAL_USER_PROGRAM;
	user_thread->context.fp = VIRTUAL_USER_STACK + USER_STACK_SIZE;
    user_thread->context.sp = VIRTUAL_USER_STACK + USER_STACK_SIZE;

    // for (int i = 0; i < filesize; i++) {
    //     user_thread->program[i] = data[i];
    // }

}

int getpid(Trap_Frame* tpf)
{
    tpf->x0 = thread_list.beg->pid;
    return thread_list.beg->pid;
}

size_t uartread(Trap_Frame *tpf,char buf[], size_t size) {
    int i = 0;
    for (int i = 0; i < size;i++) {
		#ifdef ASYNC_UART
        buf[i] = async_uart_getc();
        #else
        buf[i] = uart_getc();
        #endif
    }
    tpf->x0 = i;
    return i;
}

size_t uartwrite(Trap_Frame *tpf,const char buf[], size_t size) {
    int i = 0;
    for (i = 0; i < size; i++) {
        uart_send(buf[i]);
    }
	// async_uart_send(buf, size);
    tpf->x0 = size;
    return size;
}

int exec(Trap_Frame *tpf,const char* name, char *const argv[]) {
    disable_current_interrupt();
    jump_cpio((char*)name);
	enable_current_interrupt();

    tpf->x0 = 0;

    asm volatile("msr tpidr_el1, %0	\n": :"r"(&thread_list.beg->context));
    asm volatile("msr elr_el1, %0	\n": :"r"(thread_list.beg->context.lr));
    asm volatile("msr spsr_el1, xzr	\n");
    asm volatile("msr sp_el0, %0	\n": :"r"(thread_list.beg->context.sp));
    asm volatile("mov sp, %0	    \n": :"r"(VIRTUAL_KERNEL_STACK + KERNEL_STACK_SIZE));
    asm volatile("eret	\n");

    return 0;
}

int fork(Trap_Frame *tpf) {
    disable_current_interrupt();
	Thread *parent = thread_list.beg;
    Thread *child = thread_create(thread_list.beg->program);

    child->program = parent->program;
    child->program_size = parent->program_size;
	child->parent = parent->pid;
    int parent_pid = parent->pid;

    for (int i = 0; i < (USER_STACK_SIZE); i++) {
        child->user_stack[i] = parent->user_stack[i];
    }
    for (int i = 0; i < (KERNEL_STACK_SIZE); i++) {
        child->kernel_stack[i] = parent->kernel_stack[i];
    }
    
    store_context(get_current());
    // separate parent and child
    if(thread_list.beg->pid == parent_pid) {
        child->context = parent->context;
        // MMU
        // child->context.fp += ((unsigned long)child - (unsigned long)parent);
        // child->context.sp += ((unsigned long)child - (unsigned long)parent);

        enable_current_interrupt();

        tpf->x0 = child->pid;
        return child->pid;
    }
    // MMU
    // tpf = (Trap_Frame*)((unsigned long)tpf + (unsigned long)child - (unsigned long)parent);
    // tpf->sp_el0 += ((unsigned long)child - (unsigned long)parent);
    tpf->x0 = 0;

    return 0;
}

void exit(Trap_Frame *tpf, int status) {
    thread_list.beg->iszombie = 1;
    exec_reboot();
}

int syscall_mbox_call(Trap_Frame *tpf, unsigned char ch, unsigned int *mbox) {
    disable_current_interrupt();
    // physical mailbox address
    unsigned int *pmbox = (unsigned int *)get_low_pa(mbox);
    unsigned long r = (((unsigned long)((unsigned long)pmbox) & ~0xF) | (ch & 0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");} while (*MBOX_STATUS & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = r;
    /* now wait for the response */
    while (1) {
        /* is there a response? */
        do {
            asm volatile("nop");
        } while (*MBOX_STATUS & MBOX_EMPTY);
        /* is it a response to our message? */
        if (r == *MBOX_READ) {
            /* is it a valid successful response? */
            tpf->x0 = (mbox[1] == MBOX_RESPONSE);
            enable_current_interrupt();
            return mbox[1] == MBOX_RESPONSE;
        }
    }

    tpf->x0 = 0;
    enable_current_interrupt();
    return 0;
}

void kill(Trap_Frame *tpf, int pid) {
    Thread *killpid = thread_list.beg;
    while (killpid != 0) {
        if (killpid->pid == pid) {
            killpid->iszombie = 1;
            break;
        }
        killpid = killpid->next;
    }
}

/********
fork_test
********/

void run() {
    disable_current_interrupt();

    Thread *user_thread = thread_create(fork_test);
	user_thread->next = thread_list.beg;
	thread_list.beg = user_thread;
    thread_list.end = thread_list.beg->next;
    thread_list.end->next = NULLPTR;

	enable_current_interrupt();

    asm volatile("msr tpidr_el1, %0	\n": :"r"(&user_thread->context));
    asm volatile("msr elr_el1, %0	\n": :"r"(user_thread->context.lr));
    asm volatile("msr spsr_el1, xzr	\n");
    asm volatile("msr sp_el0, %0	\n": :"r"(user_thread->context.sp));
    asm volatile("mov sp, %0	    \n": :"r"(VIRTUAL_KERNEL_STACK + KERNEL_STACK_SIZE));
    asm volatile("eret	\n");
}

int usergetpid(){
	long ret;
	asm volatile("\
        mov x8, (0)\n\
		svc 0\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

int userfork(){
	long ret;
	asm volatile("\
        mov x8, (4)\n\
		svc 4\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

void userexit(int status){
    asm volatile("mov x8, (5)\n"::);
	asm volatile("svc 5\n"::);
    while(1);
}

int userduartwrite(char* buf,int size){
	long ret;
	asm volatile("\
        mov x8, (2)\n\
		svc 2\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

void userdelay(int cnt){
	while(cnt>0)cnt--;
}

void userprintf(char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    // we don't have memory allocation yet, so we
    // simply place our string after our code
    // char *s = (char*)&_end;
    char stmp[100];
    char *s = stmp;
    // use sprintf to format our string
    vsprintf(s,fmt,args);
    // print out as usual
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s=='\n')
            userduartwrite("\r", 1);
        userduartwrite(s++, 1);
    }
}

void fork_test(){
    userprintf("\nFork Test, pid %d\n", usergetpid());
    int cnt = 1;
    int ret = 0;
    if ((ret = userfork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        userprintf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", usergetpid(), cnt, &cnt, cur_sp);
        ++cnt;

        if ((ret = userfork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            userprintf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", usergetpid(), cnt, &cnt, cur_sp);
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                userprintf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", usergetpid(), cnt, &cnt, cur_sp);
                userdelay(1000000);
                ++cnt;
            }
        }
        userexit(0);
    }
    else {
        userprintf("parent here, pid %d, child %d\n", usergetpid(), ret);
        userexit(0);
    }
}


void foo(){
    for(int i = 0; i < 10; ++i) {
        printf("Thread id: %d %d\n", thread_list.beg->pid, i);
        delay(1000000);
        schedule();
    }
    thread_list.beg->iszombie = 1;
}