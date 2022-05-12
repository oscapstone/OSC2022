#ifndef __EXEC__H__
#define __EXEC__H__
#include "cpio.h"
#include "uart.h"

// void exec_old (cpio_new_header *header, char *file_name);

void exec_old (cpio_new_header *header, char *file_name, int enable_timer);

#endif