#include "stdio.h"
#include "stdarg.h"

void sprintf(char *dst, char *fmt, ...) {
  __builtin_va_list args;
  __builtin_va_start(args, fmt);
  vsprintf(dst, fmt, args);
}
