/*
 * stub.s - XeOS Kernel Stub
 * (c) 2002, 2003 Rink Springer, GPL
 *
 * This is a small loading stub for the kernel. It had to be written in
 * AT&T assembly syntax because NASM appearantly cannot generate usuable object
 * files for ld(1).
 *
 */
.file		"stub.s"

.text
.global __start

__start:
		/* set up all descriptors */
		mov	$0x10, %ax
		mov	%ax, %ds
		mov	%ax, %es
		mov	%ax, %fs
		mov	%ax, %gs
		mov	%ax, %ss
		mov	$0x06000, %esp

		/* XXX: This does NOT BELONG HERE !!! */
		/* turn off the floppy motors */
		mov	$0x3F2, %dx
		xor	%al,%al
		out	%al,%dx

		/* go! */
		jmp	__main

/* vim:set ts=2: */
