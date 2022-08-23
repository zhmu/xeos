/*
 * sys_module.c - XeOS Module Syscalls
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle XeOS thread syscalls.
 *
 */
#include <sys/types.h>
#include <sys/thread.h>
#include <sys/device.h>
#include <sys/filesys.h>
#include <sys/irq.h>
#include <sys/pio.h>
#include <sys/tty.h>
#include <kern/syscall.h>
#include <md/memory.h>

/*
 * module_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10)
 *
 * This will handle module syscalls.
 *
 */
uint32_t
module_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10) {
	switch (r1) {
		case SYS_module_register_irq: /* register a thread as IRQ handler */
		                              return irq_register ((uint8_t)r2, "(module)", (void*)r3, t, (uint32_t*)r4);
		case SYS_module_register_dev: /* register a device */
		                              return (uint32_t)device_register (t, (struct DEVICE*)r2);
		case SYS_module_register_pio: /* register IO space */
		                              return pio_alloc (t, (addr_t)r2, (size_t)r3);
 case SYS_module_register_lowmem: /* register low memory (XXX) */
		                              return (mapped_kernel + r2);
	  case SYS_module_register_tty: /* register tty */
		                              return tty_setdevice ((struct DEVICE*)r2);
case SYS_module_register_syscall: /* register syscall */
		                              return syscall_register (t, (char*)r2, (void*)r3);
case SYS_module_register_filesystem: /* register filesystem */
		                              return fs_register (t, (struct FILESYSTEM*)r2);
	}

	/* ??? */
	return 0;
}

/* vim:set ts=2 sw=2: */
