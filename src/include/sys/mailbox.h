/*
 * mailbox.h - XeOS Mailbox Manager
 * (c) 2003 Rink Springer, BSD licensed
 *
 * This code will handle mailboxes.
 *
 */
#ifndef __MAILBOX_H__
#define __MAILBOX_H__

#include <sys/thread.h>
#include <sys/semaphore.h>

typedef struct {
	SEMAPHORE	sem;
	size_t  numentries;
	size_t	maxentries;
	size_t	maxsize;
	struct THREAD*	wait_thread;
} MAILBOX;

MAILBOX* mailbox_create (size_t max, size_t size);
int mailbox_fetch (MAILBOX* mbox, void* msg, size_t* len);
void mailbox_add (MAILBOX* mbox, void* msg, size_t len); 
void mailbox_register (MAILBOX* mbox, struct THREAD* t);

#endif /* __MAILBOX_H__ */

/* vim:set ts=2 sw=2: */
