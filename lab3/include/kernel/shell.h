#ifndef _SHELL_H_
#define _SHELL_H_
#include "types.h"
#include "lib/print.h"
#include "lib/string.h"
#include "peripherals/mailbox.h"
#include "kernel/reboot.h"
#include "fs/initrdfs.h"
#include "kernel/timer.h"

void simple_shell(void);

#endif
