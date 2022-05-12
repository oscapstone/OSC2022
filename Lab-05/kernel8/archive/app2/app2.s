	.arch armv8-a
	.file	"app2.c"
	.text
	.section	.rodata
	.align	3
.LC0:
	.string	"app2\n"
	.align	3
.LC1:
	.string	"read test: "
	.align	3
.LC2:
	.string	"write test:"
	.align	3
.LC3:
	.string	"Fork Test, pid %d\n"
	.align	3
.LC4:
	.string	"first child pid: %d, cnt: %d, ptr: %x, sp : %x\n"
	.align	3
.LC5:
	.string	"second child pid: %d, cnt: %d, ptr: %x, sp : %x\n"
	.align	3
.LC6:
	.string	"parent here, pid %d, child %d\n"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
.LFB0:
	.cfi_startproc
	sub	sp, sp, #544
	.cfi_def_cfa_offset 544
	stp	x29, x30, [sp]
	.cfi_offset 29, -544
	.cfi_offset 30, -536
	mov	x29, sp
	adrp	x0, .LC0
	add	x0, x0, :lo12:.LC0
	bl	uart_printf
	adrp	x0, .LC1
	add	x0, x0, :lo12:.LC1
	bl	uart_printf
	add	x0, sp, 24
	mov	w1, 500
	bl	uart_read
	str	w0, [sp, 540]
	mov	w1, 500
	adrp	x0, .LC2
	add	x0, x0, :lo12:.LC2
	bl	uart_write
	add	x0, sp, 24
	ldr	w1, [sp, 540]
	bl	uart_write
	bl	getpid
	mov	w1, w0
	adrp	x0, .LC3
	add	x0, x0, :lo12:.LC3
	bl	uart_printf
	mov	w0, 1
	str	w0, [sp, 20]
	str	wzr, [sp, 536]
	bl	fork
	str	w0, [sp, 536]
	ldr	w0, [sp, 536]
	cmp	w0, 0
	bne	.L2
#APP
// 15 "app2.c" 1
	mov x0, sp
// 0 "" 2
#NO_APP
	str	x0, [sp, 528]
	bl	getpid
	mov	w5, w0
	ldr	w0, [sp, 20]
	add	x1, sp, 20
	ldr	x4, [sp, 528]
	mov	x3, x1
	mov	w2, w0
	mov	w1, w5
	adrp	x0, .LC4
	add	x0, x0, :lo12:.LC4
	bl	uart_printf
	ldr	w0, [sp, 20]
	add	w0, w0, 1
	str	w0, [sp, 20]
	bl	fork
	str	w0, [sp, 536]
	ldr	w0, [sp, 536]
	cmp	w0, 0
	beq	.L5
#APP
// 20 "app2.c" 1
	mov x0, sp
// 0 "" 2
#NO_APP
	str	x0, [sp, 528]
	bl	getpid
	mov	w5, w0
	ldr	w0, [sp, 20]
	add	x1, sp, 20
	ldr	x4, [sp, 528]
	mov	x3, x1
	mov	w2, w0
	mov	w1, w5
	adrp	x0, .LC4
	add	x0, x0, :lo12:.LC4
	bl	uart_printf
	b	.L4
.L6:
#APP
// 25 "app2.c" 1
	mov x0, sp
// 0 "" 2
#NO_APP
	str	x0, [sp, 528]
	bl	getpid
	mov	w5, w0
	ldr	w0, [sp, 20]
	add	x1, sp, 20
	ldr	x4, [sp, 528]
	mov	x3, x1
	mov	w2, w0
	mov	w1, w5
	adrp	x0, .LC5
	add	x0, x0, :lo12:.LC5
	bl	uart_printf
	mov	w0, 16960
	movk	w0, 0xf, lsl 16
	bl	delay
	ldr	w0, [sp, 20]
	add	w0, w0, 1
	str	w0, [sp, 20]
.L5:
	ldr	w0, [sp, 20]
	cmp	w0, 4
	ble	.L6
	b	.L4
.L2:
	bl	getpid
	ldr	w2, [sp, 536]
	mov	w1, w0
	adrp	x0, .LC6
	add	x0, x0, :lo12:.LC6
	bl	uart_printf
.L4:
	bl	exit
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0"
	.section	.note.GNU-stack,"",@progbits
