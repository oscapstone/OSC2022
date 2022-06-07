#include "interface.h"

        .global getpid
getpid:
        mov w8, #SYS_getpid
        svc #0
        ret

        .global uart_write
uart_write:
        mov w8, #SYS_uartwrite
        svc #0
        ret

        .global uart_read
uart_read:      
        mov w8, #SYS_uartread
        svc #0
        ret

        .global exec
exec:   
        mov w8, #SYS_exec
        svc #0
        ret
        
        .global fork
fork:
        mov w8, #SYS_fork
        svc #0
        ret

        .global exit
exit:
        mov w8, #SYS_exit
        svc #0
        ret

        .global mbox_call
mbox_call:
        mov w8, #SYS_mbox_call
        svc #0
        ret

        .global kill
kill:
        mov w8, #SYS_kill
        svc #0
        ret
