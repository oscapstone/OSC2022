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
 
#pragma once
#include "gpio.h"
#include "vfs.h"
#include <stdint.h>

#define ARM_IRQ_REG_BASE (MMIO_BASE + 0x0000b000)

#define IRQ_PENDING_1 ((volatile unsigned int *)(ARM_IRQ_REG_BASE + 0x00000204))
#define ENABLE_IRQS_1 ((volatile unsigned int *)(ARM_IRQ_REG_BASE + 0x00000210))
#define DISABLE_IRQS_1 \
  ((volatile unsigned int *)(ARM_IRQ_REG_BASE + 0x0000021c))

#define AUX_IRQ (1 << 29)

#define UART_BUFFER_SIZE 20
char read_buf[UART_BUFFER_SIZE];
char write_buf[UART_BUFFER_SIZE];
int read_buf_start, read_buf_end;
int write_buf_start, write_buf_end;

void uart_init();
void uart_send(unsigned int c);
char uart_getc();
uint32_t uart_gets(char *buf, uint32_t size);
void uart_puts(char *s);
void uart_hex(unsigned int d);
void uart_putc(char c);
void uart_int(int x);
uint32_t uart_write(char *s, uint32_t size);

void enable_uart_interrupt();
void disable_uart_interrupt();
void assert_transmit_interrupt();
void clear_transmit_interrupt();
void uart_handler();
char uart_async_getc();
void uart_async_puts(char *str);