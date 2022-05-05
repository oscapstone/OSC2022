#include <stdint.h>
#include <stdarg.h>
#include "textio.h"
#include "mini_uart.h"

void kprintf(char *fmt, ...) {
  static char temp[256];
  __builtin_va_list args;
  __builtin_va_start(args, fmt);
  vsprintf(temp,fmt,args);
  print(temp);
}

void sprintf(char *dst, char *fmt, ...) {
  __builtin_va_list args;
  __builtin_va_start(args, fmt);
  vsprintf(dst, fmt, args);
}
