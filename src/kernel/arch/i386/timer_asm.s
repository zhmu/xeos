/*
 * timer_asm.s - XeOS i386 assembly timer code
 * (c) 2002 Rink Springer, BSD licensed
 *
 * This code is heavily based on Yoctix (src/arch/i386/timer_asm.s),
 * (c) 1999 Anders Gavare.
 *
 * This code will actually handle the timer interrupt.
 *
 */

.text
.global timer_asm

timer_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %eax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs

	movb	$0x020, %al
	outb	%al, $0x20
	
	call	timer

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

/* vim:set ts=2: */
