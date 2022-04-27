#include "system.h"
#include "scheduler.h"
#include "printf.h"

int getpid(){
  __asm__ __volatile__(
    "mov x8, 0\n\t"
    "svc 0\n\t"
  );
  register int pid asm("x0");
  return pid;
}

size_t uart_read(char buf[], size_t size){
  __asm__ __volatile__(
    "mov x0, %[input0]\n\t"
    "mov x1, %[input1]\n\t"
    "mov x8, 1\n\t"
    "svc 0\n\t"
    ::[input0]"r"(buf), [input1]"r"(size)
  );
  register int read asm("x0");
  return read;
}


size_t uart_write(const char buf[], size_t size){
  __asm__ __volatile__(
    "mov x0, %[input0]\n\t"
    "mov x1, %[input1]\n\t"
    "mov x8, 2\n\t"
    "svc 0\n\t"
    ::[input0]"r"(buf), [input1]"r"(size)
  );
  register int sent asm("x0");
  return sent;
}

int exec(const char* name, char *const argv[]){
  __asm__ __volatile__(
    "mov x0, %[input0]\n\t"
    "mov x1, %[input1]\n\t"
    "mov x8, 3\n\t"
    "svc 0\n\t"
    ::[input0]"r"(name), [input1]"r"(argv)
  );
  register int value asm("x0");
  return value;
}

void exit(){
  __asm__ __volatile__(
    "mov x8, 5\n\t"
    "svc 0\n\t"
  );
}
