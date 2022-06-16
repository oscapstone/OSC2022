#ifndef _SHELL_H_
#define _SHELL_H_
#include "types.h"
#include "lib/print.h"
#include "lib/string.h"
#include "lib/simple_malloc.h"
#include "peripherals/mailbox.h"
#include "kernel/reboot.h"
#include "fs/initrdfs.h"
#include "kernel/timer.h"
#include "kernel/irq_handler.h"
#include "mm/slab.h"
#include "kernel/sched/task.h"
#include "kernel/sched/kthread.h"

void simple_shell(void);

#define DELIM " \t\n\r"
#endif
