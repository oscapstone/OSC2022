#include "start.h"
int main(int argc, char **argv) {
  for(int i = 0 ; i<10 ; i++){
    print_s("Test1, pid ");
    print_i(getpid());
    print_s("\n");
    delay(1000000000);
    // asm volatile("mov x8, #9");
    // asm volatile("svc 0");
  }
  return 0;
}