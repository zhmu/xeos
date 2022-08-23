/*
 * interrupts.c - XeOS i386 interrupts code
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code is heavily based on Yoctix (src/arch/i386/idt.c), (c) 1999
 * Anders Gavare.
 *
 * This code will handle interrupts.
 *
 */
#include <md/interrupts.h>

/*
 * arch_interrupts (int state)
 *
 * This will turn the interrupts on or off, and return the previous state.
 *
 */
int
arch_interrupts (int state) {
	volatile int flags;

	__asm__ ("pushf\npopl %%eax" : "=a" (flags));

	if (state == DISABLE)
		__asm__ ("cli");
	else
		__asm__ ("sti");

	return (flags & 0x200) ? ENABLE : DISABLE;
}

/* vim:set ts=2 sw=2: */
