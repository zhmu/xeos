/*
 * thread.h - XeOS Threads
 * (c) 2002, 2003 Rink Springer, BSD
 *
 * This is a generic include file which describes the threads.
 *
 */
#include <sys/types.h>
#if ARCH==i386
#include <md/task.h>
#endif /* ARCH==i386 */

#ifndef __THREAD_H__
#define __THREAD_H__

/* THREAD_STATUS_xxx are the status a thread can be in */
#define THREAD_STATUS_ACTIVE 1
#define THREAD_STATUS_SUSPENDED	2

/* THREAD_DEFAULT_PRIORITY is the thread's default priority */
#define THREAD_DEFAULT_PRIORITY	0x80

/* THREAD_PRIORITY_DEVICE is a device thread's priority */
#define THREAD_PRIORITY_DEVICE	0x10

/* THREAD_PRIORITY_MAILBOX is a mailbox thread's priority */
#define THREAD_PRIORITY_MAILBOX	0x20

/* THREAD_SCHEDMODE_xxx are the scheduling modes known */
#define THREAD_SCHEDMODE_NORMAL	0
#define THREAD_SCHEDMODE_PROBE	1

/*
 * THREAD describes a thread.
 *
 */
struct THREAD {
	struct THREAD* prev;                   /* previous thread */
	struct THREAD* next;                   /* next thread */
	struct THREAD* parent;                 /* parent thread */
	uint16_t status;                       /* thread status */
	uint8_t  priority;                     /* priority (lower is more) */

  /* i386 stuff below this */
#if ARCH==i386
	char*       stack;                     /* pointer to the thread's stack */
	char*       kstack;                    /* pointer to the kernel's kstack */
	uint16_t    tss_no;                    /* TSS number */
	struct TSS* tss;                       /* pointer to TSS */
#endif /* ARCH==i386 */

	/* paging handler */
	int					(*pagein)(struct THREAD*, addr_t);
};

extern struct THREAD* t_first;
extern struct THREAD* t_last;
extern struct THREAD* t_current;

#ifdef __KERNEL
void           thread_init();
struct THREAD* thread_alloc(uint16_t flags);
void           thread_switch();

void           thread_suspend (struct THREAD* t);
void           thread_resume (struct THREAD* t);

void           thread_exit (struct THREAD*);
void           thread_dump();

void           thread_mark_device(struct THREAD* t);
void           thread_mark_mailbox(struct THREAD* t);
void           thread_set_schedmode (int mode);
int            thread_isvalid (struct THREAD*);

#endif /* __KERNEL */
#endif

/* vim:set ts=2: */
