#include "syscall.h"
#include "mbox.h"
#include "uart.h"
#include "interrupt.h"
#include "address.h"
#include "memory.h"
#include "cpio.h"
#include "signal.h"
#include "mmu.h"
#include "vfs.h"
#include "string.h"
#include "dev_framebuffer.h"

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
    lock_interrupt();
    char abspath[MAX_PATHNAME];
    get_abspath(abspath, name, currThread->pwd);
   
    vnode_t *searchNode;
    if(vfs_lookup(abspath, &searchNode) == -1) return -1;
    uint64 program_size = searchNode->f_ops->getsize(searchNode);

    free(currThread->data);
    currThread->datasize = program_size;
    currThread->data = (char*)malloc(program_size);

    free_page_tables_for_thread(currThread); // free PGD
    init_page_table(&currThread->context.pgd, 0);
    set_page_tables_for_thread(currThread);
    switch_pgd((uint64)currThread->context.pgd);

    free_file_descriptor_table_for_thread(currThread);
    file_t *tmp_f;
    if(vfs_open(abspath, 0, &tmp_f) == -1) return -1;
    if(vfs_read(tmp_f, currThread->data, program_size) == -1) return -1;
    if(vfs_close(tmp_f) == -1) return -1;

    vfs_open("/dev/uart", 0, &currThread->file_descriptor_table[0]);
    vfs_open("/dev/uart", 0, &currThread->file_descriptor_table[1]);
    vfs_open("/dev/uart", 0, &currThread->file_descriptor_table[2]);

    currThread->has_signal = 0;
    for(int i = 0; i <= SIGMAX; i++) {
        currThread->signal_handlers[i] = signal_default_handlder;
    }

    // INIT_LIST_HEAD(&currThread->used_vm);
    // set_vm_list_for_thread(currThread);

    frame->elr_el1 = USER_KERNEL_BASE;
    frame->sp_el0 = USER_STACK_BASE + THREAD_STACK_SIZE;
    
    unlock_interrupt();
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

        strcpy(childThread->pwd, parentThread->pwd);
        for(int i = 0; i < MAX_FD; i++) {
            if(parentThread->file_descriptor_table[i]) {
                childThread->file_descriptor_table[i] = (file_t*)malloc(sizeof(file_t));
                *childThread->file_descriptor_table[i] = *parentThread->file_descriptor_table[i];
            }
        }

        for(int i = 0; i <= SIGMAX; i++) {
            childThread->signal_handlers[i] = parentThread->signal_handlers[i];
        }

        init_page_table(&childThread->context.pgd, 0);
        set_page_tables_for_thread(childThread);
        // set_vm_list_for_thread(childThread);
        
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



// syscall number : 11
int call_vfs_open(const char *pathname, int flags) {
    char abspath[MAX_PATHNAME];
    get_abspath(abspath, (char*)pathname, currThread->pwd);

    for(int i = 0; i < MAX_FD; i++) {
        if(!currThread->file_descriptor_table[i]) {
            if(vfs_open(abspath, flags, &currThread->file_descriptor_table[i]) == -1) {
                return -1;
            }

            return i;
        }
    }

    return -1;
}

// syscall number : 12
int call_vfs_close(int fd) {
    if(currThread->file_descriptor_table[fd]) {
        if(vfs_close(currThread->file_descriptor_table[fd]) == -1) {
            return -1;
        }
        currThread->file_descriptor_table[fd] = NULL;
        return 0;
    }

    return -1;
}

// syscall number : 13
// remember to return read size or error code
long call_vfs_write(int fd, const char *buf, unsigned long count) {
    if(currThread->file_descriptor_table[fd]) {
        return vfs_write(currThread->file_descriptor_table[fd], buf, count);
    }

    return -1;
}

// syscall number : 14
// remember to return read size or error code
long call_vfs_read(int fd, char *buf, unsigned long count) {
    if(currThread->file_descriptor_table[fd]) {
        return vfs_read(currThread->file_descriptor_table[fd], buf, count);
    }

    return -1;
}

// syscall number : 15
// you can ignore mode, since there is no access control
int call_vfs_mkdir(const char *pathname, unsigned mode) {
    char abspath[MAX_PATHNAME];
    get_abspath(abspath, (char*)pathname, currThread->pwd);

    return vfs_mkdir(abspath);
}

// syscall number : 16
// you can ignore arguments other than target (where to mount) and filesystem (fs name)
int call_vfs_mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data) {
    char abspath[MAX_PATHNAME];
    get_abspath(abspath, (char*)target, currThread->pwd);

    return vfs_mount(abspath, filesystem);
}

// syscall number : 17
int call_vfs_chdir(const char *path) {
    get_abspath(currThread->pwd, (char*)path, currThread->pwd);

    return 0;
}

// syscall number : 18
// you only need to implement seek set
long call_vfs_lseek64(int fd, long offset, int whence) {
    if(currThread->file_descriptor_table[fd]) {
        return vfs_lseek64(currThread->file_descriptor_table[fd], offset, whence);
    }

    return -1;
}

// syscall number : 19
int call_vfs_ioctl(int fd, unsigned long request, framebuffer_info_t *fb_info) {
    if(request == 0) {
        fb_info->width = width;
        fb_info->height = height;
        fb_info->pitch = pitch;
        fb_info->isrgb = isrgb;
    }

    return 0;
}