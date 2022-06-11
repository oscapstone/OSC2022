#include "syscall.h"
#include "mbox.h"
#include "uart.h"
#include "interrupt.h"
#include "address.h"
#include "memory.h"
#include "cpio.h"
#include "signal.h"
#include "mmu.h"

int getpid() {
    return currThread->pid;
}

size_t uart_read(char buf[], size_t size) {
    for(int i = 0; i < size; i++) {
        do {
            buf[i] = uart_async_getc();
        } while(buf[i] == NULL);
    }
    return size;
}

size_t uart_write(const char buf[], size_t size) {
    for(int i = 0; i < size; i++) {
        uart_putc(buf[i]);
    }

    return size;
}

int exec(trapFrame_t *frame, char* name, char *const argv[]) {
    char* program_address = load_user_program((char*)INITRAMFS_ADDR, NULL, name);
    uint64 program_size = get_program_size((char*)INITRAMFS_ADDR, name);

    currThread->datasize = program_size;
    for (int i = 0; i < program_size; i++) {
        currThread->data[i] = program_address[i];
    }   

    currThread->has_signal = 0;
    for(int i = 0; i <= SIGMAX; i++) {
        currThread->signal_handlers[i] = signal_default_handlder;
    }

    frame->elr_el1 = USER_KERNEL_BASE;
    frame->sp_el0 = USER_STACK_BASE + THREAD_STACK_SIZE;
    return 0;
}

int fork(trapFrame_t *frame) {
    lock_interrupt();
    int pid = currThread->pid;
    thread_t *parentThread = currThread;
    thread_t *childThread = createThread(parentThread->data, parentThread->datasize);     
    childThread->state = NEW_BORN;

    store_context(get_current());

    if(currThread->pid == pid) {
        for(int i = 0; i < parentThread->datasize; i++) {
            childThread->data[i] = parentThread->data[i];
        }

        for(int i = 0; i < THREAD_STACK_SIZE; i++) {
            childThread->stackPtr[i] = parentThread->stackPtr[i];
            childThread->kernel_stackPtr[i] = parentThread->kernel_stackPtr[i];
        }

        for(int i = 0; i <= SIGMAX; i++) {
            childThread->signal_handlers[i] = parentThread->signal_handlers[i];
        }

        init_page_table(&childThread->context.pgd, 0);
        // set_page_tables_for_thread(childThread);
        set_vm_list_for_thread(childThread);
        
        uint64 *temp_pgd = childThread->context.pgd;
        childThread->context = parentThread->context;
        childThread->context.pgd = temp_pgd;

        childThread->context.sp += childThread->kernel_stackPtr - parentThread->kernel_stackPtr;
        childThread->context.fp += childThread->kernel_stackPtr - parentThread->kernel_stackPtr;
        unlock_interrupt();
        return childThread->pid;
    }
    else {
        // lock_interrupt();
        // frame = (trapFrame_t*)((char *)frame + (uint64)childThread->kernel_stackPtr - (uint64)parentThread->kernel_stackPtr);
        // frame->sp_el0 += childThread->kernel_stackPtr - parentThread->kernel_stackPtr;
        // unlock_interrupt();
        return 0;
    }
}

void exit(int status) {
    thread_exit();
}

int mbox_call(unsigned char ch, unsigned int *mbox) {
    lock_interrupt();
    uint64 size = mbox[0];
    unsigned int kernel_box[36];
    memcpy(kernel_box, mbox, size);
    
    unsigned long r = (((unsigned long)((unsigned long)kernel_box) & ~0xF) | (ch & 0xF));
    // uart_printf("Mbox: 0x%x, 0x%x\n", mbox, r);
    // wait for ready 
    do {
        asm volatile("nop");
    } while(*MBOX_STATUS & MBOX_FULL);

    
    *MBOX_WRITE = r; // write the address of message to the mailbox with channel identifier
    
    while(1) {
        // wait for respose
        do {
            asm volatile("nop");
        } while(*MBOX_STATUS & MBOX_EMPTY);
        
        // make sure it is a response to our message
        if(r == PHY_TO_VIR(*MBOX_READ)) {
            // is it a valid successful response
            memcpy(mbox, kernel_box, size);
            unlock_interrupt();
            return mbox[1] == MBOX_RESPONSE;
        }
    }
    unlock_interrupt();
    return 0;
}

void kill(int pid) {
    lock_interrupt();
    if(pid <= PIDMAX && pid >= 0 && threads[pid].state == USED) {
        threads[pid].state = DEAD;
    }
    unlock_interrupt();
}

// }
// void fork_test() {
//     lock_interrupt();
//     uart_puts("\nFork Test, pid: "); uart_num(getpid()); uart_newline();
//     int cnt = 1;
//     int ret = 0;

//     if ((ret = fork(NULL)) == 0) { // child
//         long long cur_sp;
//         asm volatile("mov %0, sp" : "=r"(cur_sp));
//         uart_puts("first child pid: "); uart_num(getpid()); uart_puts(", cnt: "); uart_num(cnt);
//         uart_puts(", ptr: "); uart_hex((uint64)&cnt); uart_puts(", sp : "); uart_hex(cur_sp); uart_newline();
//         ++cnt;
        
//         if ((ret = fork(NULL)) != 0){
//             asm volatile("mov %0, sp" : "=r"(cur_sp));
//             uart_puts("first child pid: "); uart_num(getpid()); uart_puts(", cnt: "); uart_num(cnt);
//             uart_puts(", ptr: "); uart_hex((uint64)&cnt); uart_puts(", sp : "); uart_hex(cur_sp); uart_newline();
//         }
//         else{
//             while (cnt < 5) {
//                 asm volatile("mov %0, sp" : "=r"(cur_sp));
//                 uart_puts("second child pid: "); uart_num(getpid()); uart_puts(", cnt: "); uart_num(cnt);
//                 uart_puts(", ptr: "); uart_hex((uint64)&cnt); uart_puts(", sp : "); uart_hex(cur_sp); uart_newline();
//                 delay_ms(1000);
//                 ++cnt;
//             }
//         }
//     }
//     else {
//         uart_puts("parent here, pid: "); uart_num(getpid()); uart_puts(", child: "); uart_num(ret); uart_newline();
//     }

//     exit(NULL);
//     unlock_interrupt();
// }

