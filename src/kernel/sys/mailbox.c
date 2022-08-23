/*
 * mailbox.c - XeOS Mailbox Manager
 * (c) 2003 Rink Springer, BSD licensed
 *
 * This code will handle semaphores.
 *
 */
#include <sys/types.h>
#include <sys/mailbox.h>
#include <sys/semaphore.h>
#include <sys/thread.h>
#include <sys/kmalloc.h>
#include <sys/tty.h>
#include <lib/lib.h>

/*
 * mailbox_create (size_t max, size_t size)
 * 
 * This will create a new mailbox, which can store up to [max] entries of [size]
 * bytes each. It will return a pointer to the new mailbox on success or
 * NULL on failure.
 *
 */
MAILBOX*
mailbox_create (size_t max, size_t size) {
	MAILBOX* mbox = (MAILBOX*)kmalloc (NULL, sizeof (MAILBOX) + (max * (sizeof (size_t) + size)), 0);

	/* set the mailbox up */
	sem_create (&mbox->sem);
	mbox->numentries = 0;
	mbox->maxentries = max;
	mbox->maxsize = size;
	mbox->wait_thread = NULL;

	/* return the mailbox */
	return mbox;
}

/*
 * mailbox_fetch (MAILBOX* mbox, void* msg, size_t* len)
 *
 * This will fetch the first message from the mailbox, and copy it to [msg].
 * [len] will contain the length of the message. This function will return
 * zero on failure or non-zero on success.
 *
 */
int
mailbox_fetch (MAILBOX* mbox, void* msg, size_t* len) {
	size_t* sz;

	/* lock the mailbox */
	sem_get (&mbox->sem);

	/* got any entries? */
	if (mbox->numentries == 0) {
		/* no. too bad */
		sem_release (&mbox->sem);
		return 0;
	}

	/* copy the very first item */
	sz = (size_t*)(mbox + sizeof (MAILBOX));

	/* copy the length over */
	if (len)
		*len = *sz;

	/* copy the data over */
	if (msg)
		kmemcpy (msg, (void*)(mbox + sizeof (MAILBOX) + sizeof (size_t)), *sz);

	/* cycle the mailbox */
	kmemcpy ((void*)(mbox + sizeof (MAILBOX)),
	         (void*)(mbox + sizeof (MAILBOX) + (mbox->maxsize + sizeof (size_t))),
	         (mbox->maxsize + sizeof (size_t)) * (mbox->maxentries - 1));

	/* one entry less now */
	mbox->numentries--;

	/* unlock the mailbox */
	sem_release (&mbox->sem);

	/* got it */
	return 1;
}

/*
 * mailbox_add (MAILBOX* mbox, void* msg, size_t len)
 *
 * This will add [len] bytes of message [msg] to mailbox [mbox].
 *
 */
void
mailbox_add (MAILBOX* mbox, void* msg, size_t len) {
	size_t* sz;

	/* got enough space? */
	if (mbox->numentries == mbox->maxentries) {
		/* no. drop the packet */
		kprintf ("mailbox_add(): warning: dropped message for mailbox %x\n", msg);
		return;
	}

	/* lock the mailbox */
	sem_get (&mbox->sem);

	/* add the entry */
	sz = (size_t*)(mbox + sizeof (MAILBOX) + ((mbox->maxsize + sizeof (size_t)) * mbox->numentries));
	*sz = len;
	kmemcpy ((void*)(mbox + sizeof (MAILBOX) + ((mbox->maxsize + sizeof (size_t)) * mbox->numentries) + sizeof (size_t)),
	         msg,
					 len);


	/* next! */
	mbox->numentries++;

	/* wake up the thread, if any */
	if (mbox->wait_thread)
		thread_resume (mbox->wait_thread);

	/* unlock the mailbox */
	sem_release (&mbox->sem);
}

/*
 * mailbox_register (MAILBOX* mbox, struct THREAD* t)
 *
 * This will register thread [t] as mailbox handler for mailbox [mbox].
 *
 */
void
mailbox_register (MAILBOX* mbox, struct THREAD* t) {
	/* lock the mailbox */
	sem_get (&mbox->sem);

	/* update the thread */
	mbox->wait_thread = t;

	/* unlock the mailbox */
	sem_release (&mbox->sem);
}

/* vim:set ts=2 sw=2: */
