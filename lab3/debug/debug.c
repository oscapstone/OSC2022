#include "types.h"
#include "utils.h"
#include "debug/debug.h"
#include "asm.h"
extern int __bss_start, __bss_end;
extern int __text_start, __text_end;
extern int __rodata_start, __rodata_end;
extern int __data_start, __data_end;
extern int __heap_start;

#ifdef DEBUG
int debug = 1;
#else
int debug = 0;
#endif

void log_kernel_start(){
    uint64_t currentEL;
    if(debug == 0) return;
    printf("text_start=%p, text_end=%p, text_size=%u\r\n", \
           &__text_start, \
           &__text_end, \
           (uint64_t)&__text_end - (uint64_t)&__text_start
    );

 
    printf("rodata_start=%p, rodata_end=%p, rodata_size=%u\r\n", \
           &__rodata_start, \
           &__rodata_end, \
           (uint64_t)&__rodata_end - (uint64_t)&__rodata_start
    );

    printf("data_start=%p, data_end=%p, data_size=%u\r\n", \
           &__data_start, \
           &__data_end, \
           (uint64_t)&__data_end - (uint64_t)&__data_start
    );
 
    printf("bss_start=%p, bss_end=%p, bss_size=%u\r\n", \
           &__bss_start, \
           &__bss_end, \
           (uint64_t)&__bss_end - (uint64_t)&__bss_start
    );
    printf("heap_start=%p\n", &__heap_start);

    printf("current exception level: %u\r\n", get_currentEL());
    printf("SP_EL0:                  0x%x\r\n", get_SP_ELx(0));
    printf("DAIF:                    0x%x\r\n", get_DAIF());
    printf("SPSel:                   0x%x\r\n", get_SPSel());
    printf("stack pointer:           %p\r\n", &currentEL);

}
