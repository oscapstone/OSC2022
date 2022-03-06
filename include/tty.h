#ifndef TTY_H
#define TTY_H


#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif
typedef union {
    char byte[8];
    uint32_t val32;
    uint64_t val64;
} ttybuf_t;


// void tty_recv32b(ttybuf_t* buf);

// void tty_send32b(ttybuf_t* buf);
// char tty_recv8b();
// void tty_send8b(char c);

void tty_recv(ttybuf_t* buf, size_t s);
void tty_send(ttybuf_t* buf, size_t s);




#endif