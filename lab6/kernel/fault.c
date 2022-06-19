#include "kernel/fault.h"

// far save the address that trigger data abort
void fault_handler(uint64_t esr, uint64_t far){
    uint32_t ec = esr >> 26; // error code
    if(ec == FAULT_DATA_ABORT_LOW_EL || ec == FAULT_INSTR_ABORT_LOW_EL){
        do_mem_abort(esr, far);
    }else{
        INFO("unknown error: ec = %p, far: %p, esr: %p", ec, far, esr);
    }
}
