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
#ifndef __UART_H__
#define __UART_H__

#include "mbox.h"
#include "stddef.h"

//#define USE_UART0
#define READ_BUF_SIZE 64
char read_buf[READ_BUF_SIZE];
uint32_t read_buf_idx;

#define WRITE_BUF_SIZE 256
char write_buf[WRITE_BUF_SIZE];
uint32_t write_buf_in;
uint32_t write_buf_out;

void uart_init();
void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char *s);
void uart_hex(unsigned long int d);
void uart_dec(long int num);
void uart_sdec(char* pre, long int num, char* post);
void uart_shex(char* pre, unsigned long int num, char* post);


void uart_irq_handler();
void receive_handler();
void transmit_handler();
void enable_transmit_irq();
void disable_transmit_irq();
void disable_recieve_irq();
size_t uartread(char *buf, size_t size);
size_t uartwrite (const char *buf, size_t size);

#endif
