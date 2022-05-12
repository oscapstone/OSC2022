.section ".text.start"
.global _start
_start:
    mov x0, 0
1:
    sub sp, sp, 8
    add x0, x0, 1
    svc 1
    // cmp x0, 5
    b 1b
1:
    b 1b