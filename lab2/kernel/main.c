// #include "shell.h"
#include "dtb.h"
#include "stdint.h"
// #include "uart.h"
// #include "cpio.h"

void kernel_main() {
    register uint64_t x0 asm("x0");
    dtb_init(x0);
    shell();
}