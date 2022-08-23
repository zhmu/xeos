/*
 * pio.c - XeOS I/O Port Manager
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle the allocation of IO addresses.
 *
 */
#include <sys/thread.h>
#include <sys/pio.h>
#include <sys/kmalloc.h>
#include <sys/types.h>
#include <lib/lib.h>
#include <md/interrupts.h>

struct PIO* corepio = NULL;

/*
 * pio_init()
 *
 * This will initialize the PIO manager.
 *
 */
void
pio_init() {
	/* nothing to do */
}

/*
 * pio_get_owner (int start, int len)
 *
 * This will verify whether [len] IO addresses, starting at [start] are
 * allocated. It will return the owner if they are or NULL if they are not.
 *
 */
struct PIO*
pio_get_owner (int start, int len) {
	struct PIO* p = corepio;

	/* scan it all */
	while (p) {
		/* match? */
		if ((p->start >= start) && (p->start + p->count <= (start + len)))
			/* yes. return the owner */
			return p;

		/* next */
		p = p->next;
	}

	/* no match */
	return NULL;
}

/*
 * pio_alloc (struct THREAD* p, addr_t start, size_t len)
 *
 * This will allocate [len] IO addresses, beginning with [start] and assign them
 * to thread [t]. It will return 0 on success or -1 on failure.
 *
 */
int
pio_alloc (struct THREAD* t, addr_t start, size_t len) {
	struct PIO* pio = corepio;
	struct PIO* newpio;
	int oldint;

	/* are these ports already allocated? */
	if (pio_get_owner (start, len) != NULL)
		/* yes. complain */
		return 0;

	/* disable interrupts */
	oldint = arch_interrupts (DISABLE);

	/* allocate a new record and initialize it */
	newpio = (struct PIO*)kmalloc (NULL, sizeof (struct PIO), 0);
	kmemset (newpio, 0, sizeof (struct PIO));
	newpio->thread = t;
	newpio->start = start;
	newpio->count = len;

	/* search for the end of the chain */
	if (corepio)
		while (pio->next)
			pio = pio->next;

	/* do we have a single allocation? */
	if (corepio == NULL)
		/* no. this is our core pio then */
		corepio = newpio;
	else
		/* add us in the chain */
		pio->next = newpio;

	/* allow access */
	arch_thread_pio_enable (t, start, len);

	/* restore interrupts */
	arch_interrupts (oldint);

	/* all done */
	return 1;
}

/*
 * pio_dump()
 *
 * This will dump the current io administration.
 *
 */
void
pio_dump() {
	struct PIO* pio = corepio;
 
	kprintf ("PIO dump\n");
	while (pio) {
		kprintf ("  thread %x: 0x%x-0x%x\n", pio->thread, pio->start, (pio->start + pio->count));
		pio = pio->next;
	}
}

/*
 * pio_release (struct THREAD* t)
 *
 * This will release all IO adresses allocated by [t].
 *
 */
void
pio_release (struct THREAD* t) {
	struct PIO* pio = corepio;
	struct PIO* prevpio = NULL;

	/* scan them all */
	while (pio) {
		/* owned by this thread? */
		if (pio->thread == t) {
			/* yes. got a previous pio ?*/
			if (prevpio != NULL)
				/* yes. walk over this one then */
				prevpio->next = pio->next;
			else
				/* no. just replace the first one then */
				corepio = pio->next;
		}

		/* next */
		prevpio = pio; pio = pio->next;
	}
}

/* vim:set ts=2: */
