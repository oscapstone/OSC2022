#include "dtb.h"
#include "cpio.h"
#include "task.h"
#include "shell.h"
#include "buddy.h"
#include "exception.h"
#include "vfs.h"

void kernel_main(char* x0) {
    // init dtb
    dtb_init(x0);
    // init memory
    mm_init();
    // init file system
    init_rootfs();
    // init task
    task_list_init();
    // enable interrupt
    // enable_mini_uart_interrupt();
    enable_interrupt();             // enable interrupt in EL1 -> EL1
    // init cpio (unnecessary)
    cpio_init();

    shell();
}