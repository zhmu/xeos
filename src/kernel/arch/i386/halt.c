/*
 * halt.c - XeOS i386 Halting and rebooting code
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code is influenced by the OpenBSD machdep code, (c) OpenBSD
 *
 */
#include <sys/types.h>
#include <lib/lib.h>

/*
 * arch_reboot()
 *
 * This will reboot the machine.
 *
 */
void
arch_reboot() {
	unsigned char tmp[6];

	/* a farewell to interrupts */
	asm ("cli");

	/* make IDT point to the null descriptor */
	kmemset (tmp, 0, sizeof (tmp));
	asm ("lidt (%0)" : : "r" (&tmp[0]));

	/* divide by zero. since IDT is invalid, the CPU has no option but to
	   reboot... */
	asm ("div %%dx,%%ax" : : "a" (0), "b" (0));

	/* loop forever ... */
	while (1);
}

/* vim:set ts=2: */
