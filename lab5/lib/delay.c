#include "delay.h"
#include "timer.h"

void delay_s(int sec){
  unsigned long start = clock_time();
  while (clock_time() - start < sec){
    continue;
  }
}