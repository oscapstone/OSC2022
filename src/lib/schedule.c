#include "schedule.h"

thread *head, *tail;
int thread_count = 0;
dynamic_allocator *dyalloc = 0;
// int user_program_count = 0;

unsigned long user_addr;
unsigned long user_sp;

void thread_init()
{
    head = tail = 0;
    thread_count = 0;

	uart_puts("start init thread");
	
	buddy_init();
	dyalloc = dynamic_allocator_init();

	uart_puts("finish init thread");
}

void thread_test()
{
	// create first thread for context_switch
	thread *first_thread = thread_create(0);
	set_current(first_thread);
	
    for(int i = 0; i < 3; ++i)
	{
        thread_create(foo);
    }
	
    idle();

	uart_puts("finish b1");
}

void foo()
{	
	
    for (int i = 0; i < 10; ++i)
	{
		// print message, ex. Thread id: 1, index:3
        uart_puts("Thread id: ");
		uart_put_int(get_current()->pid);
		uart_puts(", index: ");
		uart_put_int(i);
		uart_puts("\n");
		
        delay(1000000);
        schedule();
    }
	
	// when finish, call exit to end of a thread
	exit();
}

void delay(int count)
{
	while (count--);
}

void exit()
{
	thread *cur_thread = get_current();
	cur_thread->thread_status = THREAD_DEAD;
	schedule();
}

void schedule()
{
    // run queue is empty
	if (head == 0)
        return;

    // run queue only first thread, free first thread, and then init
    if (head == tail && thread_count > 1) 
	{
		dynamic_free(dyalloc, (unsigned long)head);
		head = tail = 0;
		thread_count = 0;
		return;
	}

	// there are at least 2 thread in run queue, so choose cur_thread and its next to do context switch
	// at least do the context switch one time, if next thread is not TASK_RUNNING and it is put in run queue, then pass it.
	// set the cur as next, next as cur's pre
	do
	{
		tail->next = head;
		tail = head;
		head = head->next;
		tail->next = NULL;
	} while (head->thread_status != THREAD_RUNNING);

	thread*cur_thread = get_current();

	thread *next_thread = head;

	// put cur_thread and next_thread do context switch
	switch_to(cur_thread, next_thread);
}

void kill_zombies()
{
    thread* cur_thread = head;
	thread* tmp = NULL;
	
	if (cur_thread == 0) // empty 
	{
		return;
	}

	// scan all run queue to kill all zombie
    while (1)
	{           
		// cur_thread point to the last thread in run queue or next_thread is not dead          		
        while (cur_thread->next != NULL && cur_thread->next->thread_status == THREAD_DEAD) 
		{
            tmp = cur_thread->next->next;
			dynamic_free(dyalloc, (unsigned long)cur_thread->next);	
			cur_thread->next = tmp;
        }
		
		// if next thread status isn't dead, ptr = ptr->next
		if (cur_thread->next != NULL)
		{
			cur_thread = cur_thread->next;
        }
        else
		{
            tail = cur_thread;
            break;
		}
	}
}

void idle()
{
    while (1)
	{
        kill_zombies(); // reclaim threads marked as DEAD
		// do_fork(); // for requirment 2
        schedule(); // switch to any other runnable thread
		
		if (head == NULL && tail == NULL)
		{
			return;
		}
    }
}

thread* thread_create(void* func)
{
	// create new thread	
    thread* new_thread = (thread*)dynamic_alloc(dyalloc, THREAD_SIZE);	
    new_thread->context.fp = (unsigned long)new_thread + THREAD_SIZE; // frame pointer
    new_thread->context.lr = (unsigned long)func;					  // link register for function calls
    new_thread->context.sp = (unsigned long)new_thread + THREAD_SIZE; // stack pointer
    new_thread->pid = thread_count++;
    new_thread->thread_status = THREAD_RUNNING;
    new_thread->next = 0;
	
	// for requirment 2
	new_thread->program_addr = 0;
	new_thread->program_size = 0;
	new_thread->childID = 0;
	
	// add new thread to run queue
    add_to_run_queue(new_thread);
	
    return new_thread;
}

void add_to_run_queue (thread* new_thread_to_add)
{
    if (head == 0)
        head = tail = new_thread_to_add;
    else
	{
        tail->next = new_thread_to_add;
        tail = new_thread_to_add;
    }
}

/* 
	basic2 system call 
	0: int getpid()------------------------------------------
	1: size_t uartread(char buf[], size_t size)
	2: size_t uartwrite(const char buf[], size_t size)
	3: int exec(const char *name, char *const argv[])------------
	4: int fork()-----------------------------------
	5: void exit(int status)------------------------------------
	6: int mbox_call(unsigned char ch, unsigned int *mbox)
	7: void kill(int pid)
*/

// int get_pid()
// {
// 	return get_current()->pid;
// }

// int exec(char *name, char* argv[])
// {
// 	cpio_new_header *header = (cpio_new_header *)CPIO_BASE;
// 	cpio_load(header, name);
// 	thread *task = thread_create(switch_to_el0);
// 	user_addr = USER_PROGRAM_BASE;
// 	user_sp = task->context.fp;
// 	// set_video_timer();

//     idle();

// 	return 0;
// }

// void switch_to_el0() {
//     asm volatile("mov x0, 0x3c0   \n"::);
//     asm volatile("msr spsr_el1, x0   \n"::);
//     asm volatile("msr elr_el1,  %0   \n"::"r"(user_addr));
//     asm volatile("msr sp_el0,   %0   \n"::"r"(user_sp));
//     asm volatile("eret  \n"::);
// }

int fork()
{
	head->thread_status = THREAD_FORK;
	schedule();
	return head->childID;
}

// unsigned long get_user_addr()
// {
//     unsigned long addr = USER_PROGRAM_BASE + user_program_count * USER_PROGRAM_ADDR;
//     user_program_count++;
//     return addr;
// }