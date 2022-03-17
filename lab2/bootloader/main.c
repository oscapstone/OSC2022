#include "mini_uart.h"

typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

#define BOOTLOADER_ADDRESS 0x60000
#define KERNEL_ADDRESS 0x80000
#define DTB_ADDRESS 0x5FFF8
extern uint32_t __bss_end;

void kernel_main() {
  mini_uart_init();
  
  uint64_t* bld_ptr = (uint64_t*)KERNEL_ADDRESS;
  uint64_t* reloc_ptr = (uint64_t*)BOOTLOADER_ADDRESS;
  uint64_t* bld_end =(uint64_t*)(&__bss_end);
  print_hex((uint32_t)bld_ptr);
  print_char('\n');
  print_hex((uint32_t)bld_end);
  print_char('\n');
  print_hex((uint32_t)reloc_ptr);
  print_char('\n');
  while (bld_ptr != bld_end) {
    *reloc_ptr = *bld_ptr;
    bld_ptr++;
    reloc_ptr++;
  }

  // 0x20000-4 = 131068
  asm volatile("b -131068");
  // asm volatile ("b -131072");
  unsigned int size = 0;
  print("start bootloader\n");
  size = mini_uart_recv();
  size = (size << 8) | mini_uart_recv();
  size = (size << 8) | mini_uart_recv();
  size = (size << 8) | mini_uart_recv();
  print_hex(size);
  print_char('\n');

  unsigned char *kernel = (unsigned char*)(KERNEL_ADDRESS);
  unsigned char *kp = kernel;
  for (int i = 0; i < size; i++) {
    *kp++ = mini_uart_recv();
  }
  print("kernel size: ");
  print_hex(size);
  print("\nkernel address: ");
  print_hex((uint32_t)KERNEL_ADDRESS);
  
  print("\nkernel is loaded at ");
  print_hex((uint32_t)kernel);
  print(" ~ ");
  print_hex((uint32_t)(&kernel[size-1]));
  print_char('\n');
  uint32_t* keraddr = (uint32_t*) KERNEL_ADDRESS;
  uint64_t dtbaddr = *((uint64_t*) DTB_ADDRESS);
  // print_hex((uint32_t)keraddr);
  // print_char('\n');
  asm volatile("mov x0, %0"
               :
               : "r" (dtbaddr));
  asm volatile("blr %0"
               :
               : "r" (keraddr));
  // asm volatile("bl _start");
}
