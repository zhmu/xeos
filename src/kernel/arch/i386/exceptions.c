/*
 * exceptions.c - XeOS Exception Handler Code
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code is heavily based on Yoctix (src/arch/i386/idt.c), (c) 1999
 * Anders Gavare.
 *
 * This code will handle exceptions.
 *
 */
#include <sys/thread.h>
#include <sys/types.h>
#include <sys/device.h>
#include <sys/tty.h>
#include <lib/lib.h>

#define xSTACK_DUMP

/*
 * exception_handler (unsigned int no)
 *
 * This will handle exception [no].
 *
 */
void
exception_handler (uint32_t no, uint32_t ss, uint32_t gs, uint32_t fs, uint32_t es, uint32_t ds, uint32_t edi, uint32_t esi, uint32_t ebp, uint32_t esp, uint32_t ebx, uint32_t edx, uint32_t ecx, uint32_t eax, uint32_t errcode, uint32_t eip, uint32_t cs) {
#ifdef HAVE_DISASM
	int i, j;
#endif /* HAVE_DISASM */
	struct DEVICE* old_tty = tty_device;

	tty_device = NULL;

	/* figure out where the exception was */
	if (((cs & 0xffff) & 3) == 3) {
		kprintf ("userland");
	} else {
		kprintf ("kernel");
	}

	kprintf (" exception #%x at %x:%x eax=%x ebx=%x ecx=%x edx=%x esi=%x edi=%x esp=%x ebp=%x ds=%x es=%x fs=%x gs=%x ss=%x\n", no, cs & 0xffff, eip, eax, ebx, ecx, edx, esi, edi, esp, ebp, ds & 0xffff, es & 0xffff, fs & 0xffff, gs & 0xffff, ss & 0xffff);

#ifdef HAVE_DISASM
	kprintf ("Disassembly:\n");
	i = eip;
	for (j = 0; j < 5; j++) {
		kprintf ("0x%x: ", i);
		i += disasm (1, i, i, NULL);
		kprintf ("\n");
	}
#endif /* HAVE_DISASM */

	/* in kernel space? */
	if ((!(cs & 3)) || (t_current == NULL)) {
		/* yes. this is serious */
		panic ("kernel exception");
	}

	/* no. kill the thread */
	kprintf ("thread 0x%x killed\n", t_current);
	tty_device = old_tty;
	thread_exit (t_current);
}

/*
 * pagefault_handler()
 *
 * This will handle page faults.
 *
 */
void
pagefault_handler (uint32_t addr, uint32_t gs, uint32_t fs, uint32_t es, uint32_t ds, uint32_t edi, uint32_t esi, uint32_t ebp, uint32_t esp, uint32_t ebx, uint32_t edx, uint32_t ecx, uint32_t eax, uint32_t dummy, uint32_t eip, uint32_t cs) {
	struct DEVICE* old_tty = tty_device;
#ifdef STACK_DUMP
	uint32_t i, j, xaddr;
	uint32_t* sptr = (uint32_t*)esp;
#endif /* STACK_DUMP */

	/* got a thread page handler? */
	if (t_current->pagein != NULL) {
		/* yes. handle it */
		if (t_current->pagein (t_current, addr))
			/* this worked! bail out */
			return;
	}

	tty_device = NULL;
	kprintf ("pagefault_handler(): unresolvable page fault (address 0x%x) at %x:%x eax=%x ebx=%x ecx=%x edx=%x esi=%x edi=%x ebp=%x esp=%x ds=%x es=%x fs=%x gs=%x\n", addr, cs & 0xffff, eip, eax, ebx, ecx, edx, esi, edi, ebp, esp, ds & 0xffff, es & 0xffff, fs & 0xffff, gs & 0xffff);
#ifdef STACK_DUMP
	kprintf ("stack dump\n");
	xaddr = (esp - 0x40); 
	for (i = 0; i < 8; i++) {
		kprintf ("%x: ", xaddr);
		for (j = 0; j < 10; j++) {
			kprintf ("%x ", *sptr);
			sptr++; xaddr += 4;
		}
		kprintf ("\n");
	}
#endif /* STACK_DUMP */
	kprintf ("killing thread 0x%x\n", t_current);
	tty_device = old_tty;
	thread_exit (t_current);
}

/* vim:set ts=2 sw=2: */
