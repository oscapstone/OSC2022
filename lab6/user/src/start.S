.section ".text.boot"

.global _start

_start:
    bl main
    b exit

.section ".text"

.global uart_read
.global uart_write
.global getpid
.global fork
.global exec
.global exit
.global mbox_call

getpid:
    mov x8, #0
    svc 0
    ret

uart_read:
    mov x8, #1
    svc 0
    ret

uart_write:
    mov x8, #2
    svc 0
    ret

exec:
    mov x8, #3
    svc 0
    ret
    
fork:
    mov x8, #4
    svc 0
    ret

exit:
    mov x8, #5
    svc 0
    ret

mbox_call:
    mov x8, #6
    svc 0
    ret



