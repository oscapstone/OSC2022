#include <stdint.h>

#include "dtb.h"
#include "page_alloc.h"
#include "printf.h"
#include "uart.h"

void ggggg() {
    printf("GGGG" ENDL);
}

void test() {
    frame_init();
    void* a = frame_alloc(1);
    void* b = frame_alloc(2);
    // void* c = frame_alloc(1);
    // void* d = frame_alloc(1);
    // frame_free(b);
    // frame_free(c);
    // frame_free(d);
    // frame_free(a);
}

void kernel_main(char* x0) {
    timer_list_init();
    task_list_init();

    uart_enable_int(RX | TX);
    uart_enable_aux_int();
    dtb_init(x0);
    enable_interrupt();

    test();

    shell();
}