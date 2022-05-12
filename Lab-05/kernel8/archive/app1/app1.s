	.arch armv8-a
	.file	"app1.c"
	.text
	.comm	mbox,144,8
	.section	.rodata
	.align	3
.LC0:
	.string	"app1\n"
	.align	3
.LC1:
	.string	"pid %d\n"
	.align	3
.LC2:
	.string	"getBoardRevision: %d\n"
	.align	3
.LC3:
	.string	"fork_test"
	.align	3
.LC4:
	.string	"app2"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
.LFB0:
	.cfi_startproc
	stp	x29, x30, [sp, -64]!
	.cfi_def_cfa_offset 64
	.cfi_offset 29, -64
	.cfi_offset 30, -56
	mov	x29, sp
	str	w0, [sp, 28]
	str	x1, [sp, 16]
	mov	w1, 100
	adrp	x0, .LC0
	add	x0, x0, :lo12:.LC0
	bl	uart_write
	bl	getpid
	mov	w1, w0
	adrp	x0, .LC1
	add	x0, x0, :lo12:.LC1
	bl	uart_printf
	add	x0, sp, 60
	bl	getBoardRevision
	ldr	w0, [sp, 60]
	mov	w1, w0
	adrp	x0, .LC2
	add	x0, x0, :lo12:.LC2
	bl	uart_printf
	adrp	x0, .LC3
	add	x0, x0, :lo12:.LC3
	str	x0, [sp, 40]
	str	xzr, [sp, 48]
	add	x0, sp, 40
	mov	x1, x0
	adrp	x0, .LC4
	add	x0, x0, :lo12:.LC4
	bl	exec
	mov	w0, 0
	ldp	x29, x30, [sp], 64
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0"
	.section	.note.GNU-stack,"",@progbits
