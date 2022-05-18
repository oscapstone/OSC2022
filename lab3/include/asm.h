#ifndef _ASM_H_
#define _ASM_H_
extern uint32_t get_currentEL();
extern uint64_t get_SP_ELx(uint32_t);
extern uint64_t get_DAIF();
extern uint64_t get_SPSel();
extern uint64_t get_ESR_EL1();
extern uint64_t get_SPSR_EL1();
extern uint64_t get_ELR_EL1();
extern uint64_t get_SP();
#endif

