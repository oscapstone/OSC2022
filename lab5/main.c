#include "uart.h"
#include "shell.h"
#include "printf.h"
#include "malloc.h"
#include "dtb.h"
#include "timer.h"
#include "scheduler.h"
#include "cpio.h"

void main(char * arg){
  dtb_place = arg;
  uart_init();
  interrupt_enable();
  core_timer_enable();
  fdt_traverse(initramfs_callback);
  page_init();
  thread_init();
  shell();
}
