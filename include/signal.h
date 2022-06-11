#ifndef _SIGNAL_H
#define _SIGNAL_H

#include "type.h"


typedef struct trapFrame trapFrame_t;

void signal_default_handlder();
void signal_register(int code, void (*handler)());
void signal_kill(uint64 spsr_el1, int pid, int code);
void signal_execute();
void signal_user_mode();
void signal_return(trapFrame_t *frame);

#endif