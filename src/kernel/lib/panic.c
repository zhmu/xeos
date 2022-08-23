/*
 * panic.c - XeOS panic() call.
 * (c) 2002 Rink Springer, BSD licensed
 *
 * This code will handle the panic() call.
 *
 */
#include <sys/irq.h>
#include <sys/types.h>
#include <stdarg.h>
#include <lib/lib.h>
#include <md/interrupts.h>
#include <md/reboot.h>

/*
 * panic (char* fmt, ...)
 *
 * This will panic and die.
 *
 */
void
panic (char* fmt, ...) {
	va_list ap;

	/* disable interrupts. they could only mess up more */
	arch_interrupts (DISABLE);

	kprintf ("\npanic(): ");
	if (fmt != NULL) {
		va_start (ap, fmt);
		vaprintf (fmt, ap);
		va_end (va);
	}

#if 0
	kprintf ("\n\nPress any key to reboot...");

	do { __asm__ volatile ("inb %w1,%0" : "=a" (ch) : "d" (0x60)); } while (ch & 0x80);
	do { __asm__ volatile ("inb %w1,%0" : "=a" (ch) : "d" (0x60)); } while (!(ch & 0x80));
	do { __asm__ volatile ("inb %w1,%0" : "=a" (ch) : "d" (0x60)); } while (ch & 0x80);
	do { __asm__ volatile ("inb %w1,%0" : "=a" (ch) : "d" (0x60)); } while (!(ch & 0x80));

	arch_reboot();
#endif
	kprintf ("System halted");
	while (1);
}

/* vim:set ts=2 sw=2: */
