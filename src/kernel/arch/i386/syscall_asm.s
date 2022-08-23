/*
 * syscall_asm.s - XeOS i386 syscall code
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will actually handle the syscall interrupt.
 *
 */

.text
.global syscall_asm

/*
 * syscall_asm
 *
 * This is the actual syscall handler.
 *
 */
syscall_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	/* now, the stack is

		SS:ESP		=	GS
		SS:ESP + 4	=	FS
		SS:ESP + 8	=	ES
		SS:ESP + 12	=	DS
		SS:ESP + 16	=	_EDI
		SS:ESP + 20	=	_ESI
		SS:ESP + 24	=	_EBP
		SS:ESP + 28	=	_ESP
		SS:ESP + 32	=	_EBX
		SS:ESP + 36	=	_EDX
		SS:ESP + 40	=	_ECX
		SS:ESP + 44	=	_EAX
		SS:ESP + 48	=	_EIP
		SS:ESP + 52	=	_CS
		SS:ESP + 56	=	_EFLAGS
		SS:ESP + 60	=	ESP!
	*/

	mov	$0x10, %eax
	movw	%ax, %ds
	movw	%ax, %es

	mov 44(%esp),%eax
	mov 32(%esp),%ebx
	mov 40(%esp),%ecx
	mov 36(%esp),%edx
	mov 20(%esp),%esi
	mov 16(%esp),%edi

	push %edi
	push %esi
	push %edx
	push %ecx
	push %ebx
	push %eax

	call	syscall

	addl	$24, %esp

	/* change EAX back to the return value */
	mov %eax,44(%esp)

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

/* vim:set ts=2: */
