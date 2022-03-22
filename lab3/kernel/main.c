#include <stdint.h>

#include "dtb.h"

void kernel_main(char* x0) {
    dtb_init(x0);
    shell();
}