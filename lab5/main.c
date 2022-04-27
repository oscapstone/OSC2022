#include "uart.h"
#include "shell.h"
#include "printf.h"
#include "malloc.h"
#include "dtb.h"
#include "timer.h"
#include "scheduler.h"
#include "delay.h"
#include "system.h"

// extern char* dtb_place;

void foo(){
  for(int i = 0; i < 3; ++i) {
    task *cur = get_current();
    printf("Thread id: %d %d\n\r", cur->pid, i);
    delay_s(1);
    if(cur->pid == 2 && i == 1)
      exit();
    schedule();
  }
  task *cur = get_current();
  cur->state = EXIT;
  schedule();
}

void main(char * arg){
  dtb_place = arg;
  uart_init();
  __asm__ __volatile__(
    "msr DAIFClr, 0xf" // enable interrupt el1 -> el1
  ); 
  core_timer_enable();
  fdt_traverse(initramfs_callback);
  page_init();
  thread_init();
  for(int i = 0; i < 3; ++i) { // N should > 2
    task_create(foo);
  }
  idle_thread();

  shell();
}
