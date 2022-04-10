#include "uart.h"
#include "mbox.h"
#include "shell.h"
#include "string.h"
#include "malloc.h"
#include "dtb.h"
#include "cpio.h"
#include "exception.h"
#include "task.h"

void init_cpio_default_place();

extern char* dtb_place;

void main(char* dtb)
{
    //stroe dtb pointer to global (device tree)
    dtb_place = dtb;

    init_allocator();
    kmalloc(0x100000);
    //cannot use original input series after interrupt start (input is going to the buffer), use async input instead.
    //output series are not affected. (order is affected)
    task_list_init();
    enable_mini_uart_interrupt();
    enable_interrupt();  // enable interrupt in EL1 -> EL1

    init_cpio_default_place(); //stroe cpio pointer to global (file system)

    uart_printf("dtb : 0x%x\r\n",dtb);

    shell();
}

void init_cpio_default_place()
{
    traverse_device_tree(dtb_place,dtb_callback_initramfs);
}
