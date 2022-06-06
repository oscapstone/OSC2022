#include "syscall.h"
#include "../include/sched.h"
#include "peripherals/mailbox.h"
#include "fork.h"
#include "mm.h"
#include "../include/cpio.h"
#include "string.h"
#include "mmu.h"
#include "mini_uart.h"

int sys_getpid() {
    return current->id;
}

unsigned sys_uartread(char buf[], unsigned size) {
    for(unsigned int i=0; i<size; i++) {
        buf[i] = uart_recv();
    }
    return size;
}

unsigned sys_uartwrite(const char buf[], unsigned size) {
    for(int i=0; i<size; i++)
        uart_send(buf[i]);
    return size;
} 

int sys_exec(const char *name, char *const argv[]) {
    /*struct cpio_newc_header *header;
    unsigned int filesize;
    unsigned int namesize;
    unsigned int offset;
    char *filename;
    void *code_loc;

    header = DEVTREE_CPIO_BASE;
    while (1) {
        
        filename = ((void*)header) + sizeof(struct cpio_newc_header);
        
        if (stringncmp((char*)header, CPIO_HEADER_MAGIC, 6) != 0) {
            uart_send_string("invalid magic\n");
            break;
        }
        if (stringncmp(filename, CPIO_FOOTER_MAGIC, 11) == 0) {
            uart_send_string("file does not exist!\n");
            break;
        }

        namesize = hexstr_to_uint(header->c_namesize, 8);
        
        offset = sizeof(struct cpio_newc_header) + namesize;
        if (offset % 4 != 0) 
            offset = ((offset/4) + 1) * 4;

        filesize = hexstr_to_uint(header->c_filesize, 8);

        if (stringncmp(filename, name, namesize) == 0) {
            code_loc = ((void*)header) + offset;
            break;
        }

        if (filesize % 4 != 0)
            filesize = ((filesize/4) + 1) * 4;

        offset = offset + filesize;

        header = ((void*)header) + offset;
        
    }

    void *move_loc = malloc(filesize + 4096); // an extra page for bss just in case
    if(move_loc == NULL) return -1;
    for (int i=0; i<filesize; i++) {
        ((char*)move_loc)[i] = ((char*)code_loc)[i];
    }*/
    preempt_disable();

    // free old program location
    struct pt_regs *p = task_pt_regs(current);
    p->pc = 0; // move to beginning of program
    p->sp = 0xfffffffff000;

    preempt_enable();

    return -1; // only on failure*/
    // not real exec, only a restart of current process
} // fix

int sys_fork() {
    return copy_process(0, 0, 0/*, 0*/);
}

void sys_exit(int status) {
    exit_process();
}

int sys_mbox_call(unsigned char ch, volatile unsigned int *mbox) {
    printf("mbox: 0x%x\n", mbox);
    unsigned long ka_mbox = va2phys((unsigned long)mbox) + ((unsigned long)mbox&0xFFF);
    printf("mbox kernel addr location: 0x%x\n", ka_mbox);
    unsigned int r = (((unsigned int)((unsigned long)ka_mbox)&~0xF) | (ch&0xF));
    while(*MAILBOX_STATUS & MAILBOX_FULL);
    *MAILBOX_WRITE = r;
    while (1) {
        while (*MAILBOX_STATUS & MAILBOX_EMPTY) {}
        if (r == *MAILBOX_READ) {
            return ((unsigned int *)(ka_mbox+VA_START))[1]==REQUEST_SUCCEED;
        }
    }
    return 0;
}

void sys_kill(int pid) {

    struct task_struct *p;
    for (int i=0; i<NR_TASKS; i++) {
        
        if (task[i] == NULL) {
            continue;
        }

        p = task[i];
        if (p->id == (long)pid) {
            preempt_disable();
            printf("Kill target acquired.\n");
            p->state = TASK_ZOMBIE;
            //free((void *)p->stack);
            preempt_enable();
            break;
        }
        
    }

}

int sys_open(const char *pathname, int flags) {
    printf("[debug] start of syscall open\n");
    struct file *f;
    printf("[debug] test 0x%x\n", rootfs);
    printf("[debug] test 0x%x\n", rootfs->root);
    int res = vfs_open(pathname, flags, &f);
    struct pt_regs *cur_regs = task_pt_regs(current);

    if (res == FAIL) {
        cur_regs->regs[0] = -1;
        return;
    }

    int fd_num = current->files.count;

    current->files.fds[fd_num] = f;
    current->files.count++;
    cur_regs->regs[0] = fd_num;
    printf("[debug] end of syscall open\n");
}

int sys_close(int fd) {
    printf("[debug] start of syscall close\n");
    struct pt_regs *cur_regs = task_pt_regs(current);
    if (fd < 0) {
        cur_regs->regs[0] = -1;
        return;
    }
    struct file *f = current->files.fds[fd];
    cur_regs->regs[0] = vfs_close(f);
    current->files.fds[fd] = 0;
    printf("[debug] end of syscall close\n");
}

long sys_write(int fd, const void *buf, unsigned long count) {
    printf("[debug] start of syscall write\n");
    struct pt_regs *cur_regs = task_pt_regs(current);
    if (fd < 0) {
        cur_regs->regs[0] = -1;
        return;
    }
    struct file *f = current->files.fds[fd];
    if (f == 0) {
        cur_regs->regs[0] = 0;
        return;
    }
    cur_regs->regs[0] = vfs_write(f, buf, count);
    printf("[debug] end of syscall write\n");
}

long sys_read(int fd, void *buf, unsigned long count) {
    printf("[debug] start of syscall read\n");
    struct pt_regs *cur_regs = task_pt_regs(current);
    if (fd < 0) {
        cur_regs->regs[0] = -1;
        return;
    }
    struct file *f = current->files.fds[fd];
    if (f == 0) {
        cur_regs->regs[0] = 0;
        return;
    }
    cur_regs->regs[0] = vfs_read(f, buf, count);
    printf("[debug] end of syscall read\n");
}

int sys_mkdir(const char *pathname, unsigned mode) {
    printf("[debug] start of syscall mkdir\n");
    struct pt_regs *cur_regs = task_pt_regs(current);
    cur_regs->regs[0] = vfs_mkdir(pathname);
    printf("[debug] end of syscall mkdir\n");
}

int sys_mount(const char *src, const char *target, const char *fs, unsigned long flags, const void *data) {
    printf("[debug] start of syscall mount\n");
    printf("[debug] end of syscall mount\n");
}

int sys_chdir(const char *path) {
    printf("[debug] start of syscall chdir\n");
    struct pt_regs *cur_regs = task_pt_regs(current);
    cur_regs->regs[0] = vfs_chdir(path);
    printf("[debug] end of syscall chdir\n");
}

void * const sys_call_table[] =
{
    sys_getpid,
    sys_uartread,
    sys_uartwrite,
    sys_exec,
    sys_fork,
    sys_exit,
    sys_mbox_call,
    sys_kill,
    0,
    0,
    0,
    sys_open,
    sys_close,
    sys_write,
    sys_read,
    sys_mkdir,
    sys_mount,
    sys_chdir
};