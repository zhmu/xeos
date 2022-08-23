/*
 * thread.c - XeOS Thread Manager
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * The idea behind this code is heavily based on Yoctix
 * (src/sys/kern/proc.c), (c) 1999 Anders Gavare.
 *
 * This code will handle thread management.
 *
 */
#include <sys/irq.h>
#include <sys/thread.h>
#include <sys/device.h>
#include <sys/kmalloc.h>
#include <sys/types.h>
#include <sys/pio.h>
#include <lib/lib.h>
#include <md/config.h>
#include <md/interrupts.h>
#include <md/task.h>

struct THREAD* t_first = NULL;
struct THREAD* t_current = NULL;
struct THREAD* t_last = NULL;

struct THREAD* t_table;

int thread_schedmode = THREAD_SCHEDMODE_NORMAL;

#define xTSWITCH_DEBUG

/*
 * thread_init()
 *
 * This will initialize the thread mangement.
 *
 */
void
thread_init() {
	int sz = sizeof (struct THREAD) * MAX_THREADS;

	/* zap the thread table */
	t_table = (struct THREAD*)kmalloc (NULL, sz, 0);
	kmemset (t_table, 0, sz);
}

/*
 * thread_alloc(uint16_t type)
 *
 * This will allocate a new thread. It will return a pointer to the thread
 * administration on success, or NULL on failure. [type] can be a THREAD_xxx.
 *
 */
struct THREAD*
thread_alloc(uint16_t type) {
	struct THREAD* t = t_table;
	int i;

	/* scan the chain for a free TSS */
	for (i = 0; i < MAX_THREADS; i++) {
		/* is this one available? */
		if (!(t->status & THREAD_STATUS_ACTIVE))
			/* yes. stop looking */
			break;

		/* next */
		t++; i++;
	}

	/* success? */
	if (i == MAX_THREADS)
		/* no. bail out */
		return NULL;

	/* clean it out */
	kmemset (t, 0, sizeof (struct THREAD));

	/* set up the thread id and page dir */
	t->status = THREAD_STATUS_ACTIVE | type;

	/* default priority */
	t->priority = THREAD_DEFAULT_PRIORITY;

	/* let the machine dependant code do its job */
	arch_thread_init (i + 1, t);

	/* add this thread to the end of the list */
	if (t_last != NULL) {
		t_last->next = t;
		t->prev = t_last;
	} else {
		t->prev = NULL;
	}

	/* this is the last thread */
	t_last = t; t->next = NULL;

	/* do we have a first thread? */
	if (t_first == NULL)
		/* no. now, we do */
		t_first = t;

	/* return the thread */
	return t;
}

/*
 * thread_dump()
 *
 * Dumps the current thread list
 */
void
thread_dump() {
	struct THREAD* t = t_first;

	kprintf ("Thread list (t_current=0x%x):\n", t_current);
	while (t) {
		kprintf("  %x: prev=%x, next=%x, tss_no=%x  status=%x priority=%x\n",
				t, t->prev, t->next, t->tss_no, t->status, t->priority);
		t = t->next;
	}
}

/*
 * thread_isvalid (struct THREAD* t)
 *
 * This will check whether thread [t] is valid. It will return zero if not or
 * non-zero if it is.
 *
 */
int
thread_isvalid (struct THREAD* t) {
	struct THREAD* tt = t_first;

	/* walk through the entire table */
	while (tt) {
		/* match? */
		if (tt == t)
			/* yes. it's valid */
			return 1;

		/* next */
		tt = tt->next;
	}

	/* not valid, sorry */
	return 0;
}

/*
 * thread_exit (struct THREAD* t)
 *
 * This will forcefully terminate thread [t].
 *
 */
void
thread_exit (struct THREAD* t) {
	struct DEVICE* dev = coredevice;
	struct DEVICE* nextdev;
	struct THREAD* next;
	int i;
	int oldints = arch_interrupts (DISABLE);

	/* mark the table entry as available XXX */

	/* fix the chain */
	if (t->next)
		t->next->prev = t->prev;
	if (t->prev)
		t->prev->next = t->next;

	/* fix up t_first and t_last */
	if (t_first == t)
		t_first = t->next;
	if (t_last == t)
		t_last = t->prev;

	/* does this thread have a stack? */
	if (t->stack) {
		/* yes. free it */
		kfree (t->stack);
		t->stack = NULL;
	}

	/* does this task have any devices registered? */
	while (dev) {
		/* fetch the next device */
		nextdev = dev->next;

		/* are we the owning process? */
		if (dev->thread == t) {
			/* yes. unregister the device */
			kprintf ("%s: unregistered due to thread termination\n", dev->name);
			device_unregister (dev);
		}

		/* next */
		dev = nextdev;
	}

	/* scan the IRQ table for this process as well */
	for (i = 0; i <= MAX_IRQS; i++) {
		/* got a match? */
		if (irq_handlers[i].thread == t) {
			/* yes. free the IRQ */
			irq_handlers[i].thread = NULL;
			irq_handlers[i].handler = NULL;
		}
	}

	/* free any PIO ports allocated */
	pio_release (t);

	if (t->next == NULL)
		next = t_first;
	else
		next = t->next;

	/* free the machne dependant thread stuff */
	arch_thread_cleanup (t);

	/* interrupts are fine */
	arch_interrupts (oldints);

	/* is this the current process? */
	if (t_current == t) {
		/* yes. force a switch */
		t->status = THREAD_STATUS_SUSPENDED;
		thread_switch ();
	}
}

/*
 * thread_switch_probe()
 *
 * THis will handle task switches in probe scheduling mode.
 *
 */
void
thread_switch_probe() {
	/* got a current thread? */
	if (t_current == NULL) {
		/* no. start from the first thread */
		t_current = t_first;

		/* jump to the thread */
		arch_thread_launch (t_current);
		return;
	}

	/* is the current thread suspended? */
	if (!(t_current->status & THREAD_STATUS_SUSPENDED))
		/* no. don't switch */
		return;

	/* next thread */
	t_current = t_current->next;

	/* at the final one? */
	if (t_current->next == NULL) {
		/* yes. switch the multitasker */
		thread_schedmode = THREAD_SCHEDMODE_NORMAL;
	}

	/* jump to the thread */
	arch_thread_launch (t_current);
}

/*
 * thread_switch_normal()
 *
 * This will switch to the next process, using the normal scheduler.
 *
 */
void
thread_switch_normal() {
	struct THREAD* t_new;
	struct THREAD* t_tmp = t_first;
	struct THREAD* t_maxprio = NULL;
	uint8_t curprio = THREAD_DEFAULT_PRIORITY;

	/* scan the chain for a thread with the most priority */
	while (t_tmp) {
		/* got a not-suspended thread with more priority? */
		if ((!(t_tmp->status & THREAD_STATUS_SUSPENDED)) &&
				(t_tmp->priority < curprio)) {
			/* yes. mark it */
			t_maxprio = t_tmp;
			curprio = t_maxprio->priority;
		}

		/* next */
		t_tmp = t_tmp->next;
	}

	/* got a priority thread? */
	if (t_maxprio != NULL) {
		/* yes. is this the current thread? */
		if (t_current == t_maxprio)
			/* yes. just leave */
			return;

		/* no. switch to it */
		t_current = t_maxprio;
		arch_thread_launch (t_current);
		return;
	}

	/* do we have a current task? */
	if (t_current == NULL) {
		/* no. what about a first task? */
		if (t_first == NULL)
			/* nope. get out of here then */
			return;

		/* switch to the very first task */
		t_new = t_first;
	} else {
		/* switch to next task */
		if (t_current->next != NULL) {
			t_new = t_current->next;
		} else {
			t_new = t_first;
		}
	}

#ifdef TSWITCH_DEBUG
	kprintf ("thread_switch(): called with 0x%x, new thread is 0x%x\n", new, t_new);
#endif /* TSWITCH_DEBUG */


	/* about to switch to a NULL task? */
	if (t_new == NULL) {
		/* yes. activate the first task */
		t_new = t_first;
		if (t_new == NULL)
			panic ("thread_switch(): t_first is NULL!\n");
	}

	/* is the task we're about to execute suspended? */
	while (t_new->status & THREAD_STATUS_SUSPENDED) {
		/* yes. follow the chain until we have one which is not */
		t_new = t_new->next;

	/* start at the begin if neccesary */
	if (t_new == NULL)
	   t_new = t_first;
	}

	/* got a new task? */
	if (t_new != t_current) {
		/* yes. perform the switch */
		t_current = t_new;
#ifdef TSWITCH_DEBUG
		kprintf ("thread_switch(): doing switch to 0x%x\n", t_current);
#endif /* TSWITCH_DEBUG */

		arch_thread_launch (t_current);
	}
}

/*
 * thread_switch()
 *
 * This will handle thread switches.
 *
 */
void
thread_switch() {
	switch (thread_schedmode) {
		case THREAD_SCHEDMODE_NORMAL: thread_switch_normal();
																	return;
		 case THREAD_SCHEDMODE_PROBE: thread_switch_probe();
																	return;
	}
	panic ("thread_switch(): unknown scheduling mode");
}

/*
 * thread_suspend (struct THREAD* t)
 *
 * This will suspend thread [t].
 *
 */
void
thread_suspend (struct THREAD* t) {
	/* set the flag */
	t->status |= THREAD_STATUS_SUSPENDED;

	/* current thread? */
	if (t_current == t)
		/* yes. switch */ 
		thread_switch ();
}

/*
 * thread_resume (struct THREAD* t)
 *
 * This will resume thread [t].
 *
 */
void
thread_resume (struct THREAD* t) {
	t->status &= ~THREAD_STATUS_SUSPENDED;
}

/*
 * thread_mark_device(struct THREAD* t)
 *
 * This will mark thread [t] as being a device thread.
 *
 */
void
thread_mark_device(struct THREAD* t) {
	t->priority = THREAD_PRIORITY_DEVICE;
}

/*
 * thread_mark_mailbox(struct THREAD* t)
 *
 * This will mark thread [t] as being a mailbox thread.
 *
 */
void
thread_mark_mailbox(struct THREAD* t) {
	t->priority = THREAD_PRIORITY_MAILBOX;
}

/*
 * thread_set_schedmode (int mode)
 *
 * This will change the scheduling mode to [mode].
 *
 */
void
thread_set_schedmode (int mode) {
	thread_schedmode = mode;
}

/* vim:set ts=2 sw=2: */
