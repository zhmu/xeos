/*
 * main.c - XeOS Kernel Main
 *
 * Here the fun begins!
 *
 */
#include <sys/types.h>
#include <sys/kmalloc.h>
#include <sys/network.h>
#include <sys/elf.h>
#include <sys/thread.h>
#include <sys/tty.h>
#include <sys/pio.h>
#include <sys/irq.h>
#include <kern/syscall.h>
#include <lib/lib.h>
#include <md/memory.h>
#include <md/init.h>
#include "../version.h"

struct sMODULERESOURCES modconsole_res = { 0x60, 1, 0, 0 };
struct sMODULECONFIG modconsole_config = { "console0", NULL, &modconsole_res };

struct sMODULERESOURCES modne_res = { 0x300, 10, 0, 0 };
struct sMODULECONFIG modne2k_config = { "ne0", NULL, &modne_res };

struct sMODULERESOURCES modide_res = { 0x1f0, 14, 0, 0 };
struct sMODULECONFIG modide_config = { "ide0", NULL, &modide_res };

#include "../../modules/modules.c"

/*
 * kmain()
 *
 * This is the 'main' function for the kernel. It will be run in Ring 0.
 *
 */
void
kmain() {
	struct THREAD* t;
	size_t total, avail;

	/* initialize the tty */
	tty_init();

	/* initialize the memory allocator and add all available memory */
	kmalloc_init();
	arch_add_all_memory();

	/* initialize the irq manager */
	irq_init();

	/* initialize machine dependant stuff */
	arch_init();

	/* initialize the io manager */
	pio_init();

	/* initialize networking */
	network_init();

	/* initialize the device chain */
	device_init();

	/* be verbose */
	kprintf (VERSION"\n\n");
	kmemstats (&total, &avail);
	kprintf ("Memory: %u KB total, %u KB available\n", (total / 1024), (avail / 1024));

	/* console driver */
	t = thread_alloc (0);
	kprintf ("console is %x\n", t);
	elf_parse (ELF_console, ELF_console_size, t);
	arch_thread_push (t, (uint32_t)&modconsole_config);

	/* ipv4 */
	t = thread_alloc (0);
	kprintf ("ipv4 is %x\n", t);
	elf_parse (ELF_ipv4, ELF_ipv4_size, t);
	arch_thread_push (t, NULL);

	/* ne2000 */
	t = thread_alloc (0);
	kprintf ("ne2000 is %x\n", t);
	elf_parse (ELF_ne2k, ELF_ne2k_size, t);
	arch_thread_push (t, (uint32_t)&modne2k_config);

	/* ide */
	t = thread_alloc (0);
	kprintf ("ide is %x\n", t);
	elf_parse (ELF_ide, ELF_ide_size, t);
	arch_thread_push (t, (uint32_t)&modide_config);
	
	/* fatfs */
	t = thread_alloc (0);
	kprintf ("fatfs is %x\n", t);
	elf_parse (ELF_fatfs, ELF_fatfs_size, t);
	arch_thread_push (t, NULL);

	/* sh */
	t = thread_alloc (0);
	kprintf ("sh is %x\n", t);
	elf_parse (ELF_sh, ELF_sh_size, t);
	arch_thread_push (t, NULL);

	/* activate the probe mode of the scheduler */
	thread_set_schedmode (THREAD_SCHEDMODE_PROBE);

	/* launch the first thread, the scheduler will do the rest */
	thread_switch();
	
	/* NOTREACHED */
}

/* vim:set ts=2 sw=2: */
