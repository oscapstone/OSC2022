/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */
#include "stddef.h"
#include "uart.h"
#include "cpio.h"
#include "shell.h"
#include "string.h"
#include "dtb.h"
#include "stdlib.h"
#include "buddy.h"
#include "math.h"
#include "mm.h"


void main()
{
    // read device tree address from x0
    register int *reg_x0 asm("x0");
    DTB_BASE = reg_x0;
    
    // set up serial console
    uart_init();
    set_timer_interrupt(false);
    dtb_parse(cpio_init);
    
    init_buddy();
    uart_puts("Reserve spin table\n");
    reserve_memory(0xFFFF000000000000, 0xFFFF000000001000);
    uart_puts("Reserve kernel image\n");
    reserve_memory(0xFFFF000000060000, 0xFFFF000000100000);
    uart_puts("Reserve initfs\n");
    // uart_hex(CPIO_BASE);
    // uart_hex(CPIO_END);
    reserve_memory(CPIO_BASE, CPIO_END);
    uart_puts("Reserve devicetree\n");
    reserve_memory(DTB_BASE, DTB_END);
    uart_puts("Reserve simple_alloc\n");
    reserve_memory(heap_start, heap_start + heap_size);

    build_free_list();
    build_buddy();
    mm_init();
    thread_module_init();
    
    set_start_time();

    uart_shex("frame_arr: 0x", frame_arr, "\n");
    uart_shex("test PAGEADDR: 0x", PAGE2ADDR(5),"\n");
    shell();
    
}
