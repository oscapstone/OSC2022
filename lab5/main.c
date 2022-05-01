#include "uart.h"
#include "shell.h"
#include "printf.h"
#include "malloc.h"
#include "dtb.h"
#include "timer.h"
#include "scheduler.h"
#include "delay.h"
#include "system.h"
#include "cpio.h"
#include "mailbox.h"

// extern char* dtb_place;

void main(char * arg){
  dtb_place = arg;
  interrupt_enable();
  uart_init();
  core_timer_enable();
  fdt_traverse(initramfs_callback);
  page_init();

  thread_init();
  idle_thread();
  shell();
}
