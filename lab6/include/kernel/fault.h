#ifndef _FAULT_H_
#define _FAULT_H_

#include "mm/mm.h"
#include "lib/print.h"
#include "types.h"

#define FAULT_DATA_ABORT_LOW_EL     (0b100100)
#define FAULT_DATA_ABORT            (0b100101)
#define FAULT_INSTR_ABORT_LOW_EL    (0b100000)
#define FAULT_INSTR_ABORT           (0b100001)

#endif
