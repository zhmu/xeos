/*
 * sys_io.c - XeOS IO Syscalls
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle XeOS IO syscalls.
 *
 */
#include <sys/types.h>
#include <kern/syscall.h>
#include <sys/thread.h>
#include <sys/irq.h>
#include <sys/tty.h>

/*
 * io_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10)
 *
 * This will handle IO syscalls.
 *
 */
uint32_t
io_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10) {
	switch (r1) {
		case SYS_io_putchar: /* put a char on the screen */
		                     tty_putchar ((uint8_t)r2);
												 return 1;
		case SYS_io_getchar: /* fetch a char */
												 return tty_getchar();
		case SYS_io_devread: /* read from a device */
												 return device_read ((struct DEVICE*)r2, (size_t)r3, (void*)r4, (size_t)r5);
	}

	/* ??? */
	return 0;
}

/* vim:set ts=2 sw=2: */
