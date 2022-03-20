#include "uart.h"
#include "mbox.h"
#include "shell.h"
#include "string.h"
#include "malloc.h"
#include "dtb.h"
#include "cpio.h"

void init_cpio_default_place();

extern char* dtb_place;

void main(char* dtb)
{
    //stroe dtb pointer to global (device tree)
    dtb_place = dtb;
    enable_mini_uart_interrupt();
    init_cpio_default_place(); //stroe cpio pointer to global (file system)

    uart_printf("dtb : 0x%x\n",dtb);

    shell();
}

void init_cpio_default_place()
{
    traverse_device_tree(dtb_place,dtb_callback_initramfs);
}
