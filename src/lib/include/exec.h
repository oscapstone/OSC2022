#ifndef __EXEC__H__
#define __EXEC__H__

#include "cpio.h"
#include "timer.h"

void exec(cpio_new_header *header ,char *fpath, int enable_timer);

#endif