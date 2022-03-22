#include <stdint.h>

#include "dtb.h"
#include "timer.h"

void kernel_main(char* x0) {
    dtb_init(x0);

    enable_core_timer();
    shell();
}