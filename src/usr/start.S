.section ".text.boot"

.global _start

_start:
    bl main
    b  hang

hang:  
    wfe
    b  hang

.globl getpid
getpid:
	mov x8, 0
    svc #0
    ret

.globl uartread
uartread:
	mov x8, 1
    svc #0
    ret

.globl uartwrite
uartwrite:
	mov x8, 2
    svc #0
    ret

.globl exec
exec:
	mov x8, 3
    svc #0
    ret

.globl fork
fork:
	mov x8, 4
    svc #0
    ret