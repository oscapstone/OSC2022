#include "syscall.h"
#include "sched.h"
#include "stddef.h"
#include "uart.h"
#include "filesystem.h"
#include "exception.h"
#include "malloc.h"
#include "mbox.h"
#include "signal.h"
#include "mmu.h"
#include "string.h"

int getpid(trapframe_t* tpf)
{
    tpf->x0 = curr_thread->pid;
    return curr_thread->pid;
}

size_t uartread(trapframe_t *tpf,char buf[], size_t size)
{
    int i = 0;
    for (int i = 0; i < size;i++)
    {
        buf[i] = uart_getc();
    }
    tpf->x0 = i;
    return i;
}

size_t uartwrite(trapframe_t *tpf,const char buf[], size_t size)
{
    int i = 0;
    for (int i = 0; i < size; i++)
    {
        uart_putc(buf[i]); // TODO : debug -> some unknown bugs occur when uart_async_putc (only in qemu)
    }
    tpf->x0 = i;
    return i;
}

//In this lab, you won’t have to deal with argument passing
int exec(trapframe_t *tpf,const char *name, char *const argv[])
{
    // free alloced area and vma struct
    // free alloced area and vma struct
    list_head_t *pos = curr_thread->vma_list.next;
    while (pos != &curr_thread->vma_list)
    {
        if (((vm_area_struct_t *)pos)->is_alloced)
            kfree((void *)PHYS_TO_VIRT(((vm_area_struct_t *)pos)->phys_addr));

        list_head_t *next_pos = pos->next;
        kfree(pos);
        pos = next_pos;
    }

    INIT_LIST_HEAD(&curr_thread->vma_list);
    curr_thread->datasize = get_file_size((char *)name);
    char *new_data = get_file_start((char *)name);
    curr_thread->data = kmalloc(curr_thread->datasize);
    curr_thread->stack_alloced_ptr = kmalloc(USTACK_SIZE);

    asm("dsb ish\n\t");      // ensure write has completed
    free_page_tables(curr_thread->context.ttbr0_el1, 0);
    memset(PHYS_TO_VIRT(curr_thread->context.ttbr0_el1), 0, 0x1000);
    asm("tlbi vmalle1is\n\t" // invalidate all TLB entries
        "dsb ish\n\t"        // ensure completion of TLB invalidatation
        "isb\n\t");          // clear pipeline

    // remap code
    add_vma(curr_thread, 0, curr_thread->datasize, (size_t)VIRT_TO_PHYS(curr_thread->data), 7,1);
    // remap stack
    add_vma(curr_thread, 0xffffffffb000, 0x4000, (size_t)VIRT_TO_PHYS(curr_thread->stack_alloced_ptr), 7, 1);
    // device
    add_vma(curr_thread, 0x3C000000L, 0x3000000L, 0x3C000000L, 3, 0);
    // for signal wrapper
    add_vma(curr_thread, USER_SIG_WRAPPER_VIRT_ADDR_ALIGNED, 0x2000, (size_t)VIRT_TO_PHYS(signal_handler_wrapper), 5, 0);

    //copy file into data
    memcpy(curr_thread->data, new_data, curr_thread->datasize);

    //clear signal handler
    for (int i = 0; i <= SIGNAL_MAX; i++)
    {
        curr_thread->singal_handler[i] = signal_default_handler;
    }

    tpf->elr_el1 = 0;
    tpf->sp_el0 = 0xfffffffff000;
    tpf->x0 = 0;
    return 0;
}

int fork(trapframe_t *tpf)
{
    lock();
    thread_t *newt = thread_create(curr_thread->data,curr_thread->datasize);

    //copy signal handler
    for (int i = 0; i <= SIGNAL_MAX;i++)
    {
        newt->singal_handler[i] = curr_thread->singal_handler[i];
    }

    list_head_t *pos;
    list_for_each(pos, &curr_thread->vma_list){

        // ignore device and signal wrapper
        if (((vm_area_struct_t *)pos)->virt_addr == USER_SIG_WRAPPER_VIRT_ADDR_ALIGNED || ((vm_area_struct_t *)pos)->virt_addr == 0x3C000000L)
        {
            continue;
        }

        char *new_alloc = kmalloc(((vm_area_struct_t *)pos)->area_size);
        add_vma(newt, ((vm_area_struct_t *)pos)->virt_addr, ((vm_area_struct_t *)pos)->area_size, (size_t)VIRT_TO_PHYS(new_alloc), ((vm_area_struct_t *)pos)->rwx, 1);

        memcpy(new_alloc, (void*)PHYS_TO_VIRT(((vm_area_struct_t *)pos)->phys_addr), ((vm_area_struct_t *)pos)->area_size);
    }

    // device
    add_vma(newt, 0x3C000000L, 0x3000000L, 0x3C000000L, 3, 0);
    // for signal wrapper
    add_vma(newt, USER_SIG_WRAPPER_VIRT_ADDR_ALIGNED, 0x2000, (size_t)VIRT_TO_PHYS(signal_handler_wrapper), 5, 0); // for signal wrapper

    int parent_pid = curr_thread->pid;

    //copy stack into new process
    for (int i = 0; i < KSTACK_SIZE; i++)
    {
        newt->kernel_stack_alloced_ptr[i] = curr_thread->kernel_stack_alloced_ptr[i];
    }

    store_context(get_current());
    //for child
    if( parent_pid != curr_thread->pid)
    {
        goto child;
    }

    void *temp_ttbr0_el1 = newt->context.ttbr0_el1;
    newt->context = curr_thread->context;
    newt->context.ttbr0_el1 = VIRT_TO_PHYS(temp_ttbr0_el1);
    newt->context.fp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr; // move fp
    newt->context.sp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr; // move kernel sp

    unlock();

    tpf->x0 = newt->pid;
    return newt->pid;

child:
    tpf->x0 = 0;
    return 0;
}

void exit(trapframe_t *tpf, int status)
{
    thread_exit();
}

int syscall_mbox_call(trapframe_t *tpf, unsigned char ch, unsigned int *mbox_user)
{
    //mbox_user = (unsigned int *)(curr_thread->stack_alloced_ptr + USTACK_SIZE - (0xfffffffff000 - (size_t)mbox));
    lock();

    unsigned int size_of_mbox = mbox_user[0];
    memcpy((char *)mbox, mbox_user, size_of_mbox);
    mbox_call(ch);
    memcpy(mbox_user, (char *)mbox, size_of_mbox);

    tpf->x0 = 8;
    unlock();
    return 0;
}

void kill(trapframe_t *tpf,int pid)
{
    lock();
    if (pid >= PIDMAX || pid < 0  || !threads[pid].isused)
    {
        unlock();
        return;
    }
    threads[pid].iszombie = 1;
    unlock();
    schedule();
}

void signal_register(int signal, void (*handler)())
{
    if (signal > SIGNAL_MAX || signal < 0)return;

    curr_thread->singal_handler[signal] = handler;
}

void signal_kill(int pid, int signal)
{
    if (pid > PIDMAX || pid < 0 || !threads[pid].isused)return;

    lock();
    threads[pid].sigcount[signal]++;
    unlock();
}

void sigreturn(trapframe_t *tpf)
{
    load_context(&curr_thread->signal_saved_context);
}

//only need to implement the anonymous page mapping in this Lab.
void *sys_mmap(trapframe_t *tpf, void *addr, size_t len, int prot, int flags, int fd, int file_offset)
{
    // relocate to zero
    if (len + (unsigned long)addr >= 0xfffffffff000L)addr = 0L;

    len = len % 0x1000 ? len + (0x1000 - len % 0x1000) : len; // rounds up
    addr = (unsigned long)addr%0x1000?addr + (0x1000 - (unsigned long)addr % 0x1000):addr;

    // check if overlap
    list_head_t *pos;
    vm_area_struct_t *the_area_ptr = 0;
    list_for_each(pos, &curr_thread->vma_list)
    {
        if (!(((vm_area_struct_t *)pos)->virt_addr >= (unsigned long)(addr + len) || ((vm_area_struct_t *)pos)->virt_addr + ((vm_area_struct_t *)pos)->area_size <= (unsigned long)addr))
        {
            the_area_ptr = (vm_area_struct_t *)pos;
            break;
        }
    }

    // test the end of the area as addr
    if (the_area_ptr)
    {
        tpf->x0 = (unsigned long)sys_mmap(tpf, (void *)(the_area_ptr->virt_addr + the_area_ptr->area_size), len, prot, flags, fd, file_offset);
        return (void *)tpf->x0;
    }

    add_vma(curr_thread, (unsigned long)addr, len, VIRT_TO_PHYS((unsigned long)kmalloc(len)), prot, 1);
    tpf->x0 = (unsigned long)addr;
    return (void*)tpf->x0;
}
