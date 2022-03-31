#ifndef SHELL_H
#define SHELL_H

#include "utils.h"
#include "uart.h"
#include "system.h"
#include "irq.h"


#define  USER_NAME     "ricky"
#define  MACHINE_NAME  "pi"


void shell_welcome();
void shell() ;

#endif