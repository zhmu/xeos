/*
 * XeOS Kernel Memory Allocator - (c) 2003 Rink Springer
 *
 * This code is inspired by Yoctix, (c) 1999 Anders Gavare.
 *
 */
#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include <sys/types.h>
#include <md/config.h>

/* KMALLOC_BITMAPSIZE is the size of an entry within a bitmap, and
 * (PAGESIZE % KMALLOC_BITMAPSIZE) must be zero */
#define KMALLOC_BITMAPSIZE	16

/* KMALLOC_CFLAGS_USED means a chunk is in use */
#define KMALLOC_CFLAGS_USED	1

/* KMALLOC_CFLAGS_BITMAP means a chunk holds a bitmap */
#define KMALLOC_CFLAGS_BITMAP	2

/* KMALLOC_CFLAGS_CHAIN means a chunk is in a chain of chunks */
#define KMALLOC_CFLAGS_CHAIN 4

/*
 * KMALLOC_REGION describes a region of memory which can be allocated.
 *
 */
struct KMALLOC_REGION {
	/* address is the plain address of the memory */
	addr_t	address;

	/* numchunks is the number of chunks */
	size_t	numchunks;

	/* size is the size of the region, in pages */
	size_t	size;

	/* avail is the available memory, in pages */
	size_t	avail;

	/* flags are possible region flags */
	uint32_t	flags;

	/* next is a pointer to the next region */
	struct KMALLOC_REGION* next;
};

/*
 * KMALLOC_CHUNK describes a chunk of memory, which resides somewhere within
 * a region.
 *
 */
struct KMALLOC_CHUNK {
	/* address is the plain address of the actual memory */
	addr_t	address;

	/* flags are possible chunk flags */
	uint32_t	flags;

	union {
		/* chain is the number of chunks in this allocation chain */
		size_t	chain;

		/* bitmap_left is the number of bitmap-chunks left */
		size_t	bitmap_left;
	};

	/* t is the thread which owns this chunk */
	struct THREAD* thread;
};

#ifdef __KERNEL
void	kmalloc_init();
void* kmalloc (struct THREAD* t, size_t size, uint32_t flags);
void  kmalloc_addregion (addr_t addr, size_t size, uint32_t flags);
void  kfree (void* ptr);
void	kmemstats(size_t* total, size_t* avail);
#endif /* __KERNEL */

#endif /* __KMALLOC_H__ */

/* vim:set ts=2 sw=2: */
