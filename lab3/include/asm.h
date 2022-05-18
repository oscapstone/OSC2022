#ifndef _ASM_H_
#define _ASM_H_
uint32_t get_currentEL();
uint64_t get_SP_ELx(uint32_t);
uint64_t get_DAIF();
uint64_t get_SPSel();
#endif

