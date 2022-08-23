/*
 * pio.h - XeOS I/O Port Manager
 * (c) 2002 Rink Springer, BSD licensed
 *
 * This code will handle the allocation of IO addresses.
 *
 */
#include <sys/types.h>

#ifndef __PIO_H__
#define __PIO_H__

struct PIO {
	struct PIO*	 next;			/* next */

	struct THREAD* thread;  /* owning proces */
	addr_t         start;   /* starting address */
	size_t         count;   /* number of addresses */
};

#ifdef __KERNEL
void pio_init();
int pio_alloc (struct THREAD* t, addr_t start, size_t len);
void pio_release (struct THREAD* t);
void pio_dump();
#endif /* __KERNEL */

#endif /* __PIO_H__ */

/* vim:set ts=2 sw=2: */
