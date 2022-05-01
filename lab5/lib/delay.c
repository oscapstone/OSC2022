#include "delay.h"
#include "timer.h"
#include "printf.h"


void delay_s(uint32_t sec){
  unsigned long start = clock_time_s();
  while (clock_time_s() - start < sec){
    continue;
  }
}

void delay_tick(uint64_t tick){
  do{
    asm volatile("nop");
  }while(tick--);
}
