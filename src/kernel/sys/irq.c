/*
 * irq.c - XeOS Interrupt Manager
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code is inspired by Yoctix (src/sys/reg/interrupts.c), (c) 1999
 * Anders Gavare.
 *
 * This code will handle the allocation of interrupts.
 *
 */
#include <sys/types.h>
#include <sys/irq.h>
#include <sys/thread.h>
#include <lib/lib.h>
#include <md/config.h>
#include <md/interrupts.h>

IRQ_HANDLER irq_handlers[MAX_IRQS + 1];

/*
 * irq_init()
 *
 * This will initialize the IRQ manager.
 *
 */
void
irq_init() {
	int i;

	/* reset all handlers */
	for (i = 0; i <= MAX_IRQS; i++) {
		irq_handlers[i].name = NULL;
		irq_handlers[i].handler = NULL;
		irq_handlers[i].thread = NULL;
		irq_handlers[i].var = NULL;
		irq_handlers[i].stray_count = 0;
	}
}

/*
 * irq_register (unsigned int num, char* name, void* handler, struct THREAD* t,
 *               uint32_t* var)
 *
 * This will assign handler [handler], thread [t] or variable [var] to IRQ
 * [num]. [name] is used as a description. This will return zero on failure on
 * non-zero on success. If [var] is not NULL, it will be used as the refence
 * value. If [p] is not NULL, [handler] is ignored and [p] will be resumed if
 * the interrupt occours.
 *
 */
int
irq_register (uint8_t num, char* name, void* handler, struct THREAD* t, uint32_t* var) {
	int oldints;

	/* number within range? */
	if (num > MAX_IRQS)
		/* no. complain */
		return 0;

	/* is this irq already used by anyone but us ? */
	if ((irq_handlers[num].thread != NULL) && (irq_handlers[num].thread != t) &&
	    (irq_handlers[num].var != NULL))
		/* this failed. complain */
		return 0;

	/* update the entry */
	oldints = arch_interrupts (DISABLE);
	irq_handlers[num].name = name;
	irq_handlers[num].handler = handler;
	irq_handlers[num].thread = t;
	irq_handlers[num].var = var;
	irq_handlers[num].stray_count = 0;
	arch_interrupts (oldints);

	/* all went ok */
	return 1;
}

/*
 * irq_unregister (uint8_t num)
 *
 * This will unregister IRQ [num].
 *
 */
void
irq_unregister (uint8_t num) {
	int oldints;

	/* number within range? */
	if (num > MAX_IRQS)
		/* no. complain */
		return;

	/* disable interrupts and zap the handler */
	oldints = arch_interrupts (DISABLE);
	irq_handlers[num].name = NULL;
	irq_handlers[num].handler = NULL;
	irq_handlers[num].thread = NULL;
	irq_handlers[num].var = NULL;
	irq_handlers[num].stray_count = 0;
	arch_interrupts (oldints);
}

/*
 * irq_handler (unsigned int num)
 *
 * This will run the IRQ handler of IRQ [num].
 *
 */
void
irq_handler (unsigned int num) {
	void (*handler)() = irq_handlers[num].handler;
	struct THREAD* t = irq_handlers[num].thread;
	uint32_t* var = irq_handlers[num].var;

	/* is this IRQ properly handled? */
	if ((handler == NULL) && (t == NULL) && (var == NULL)) {
		/* no. got enough? */
		if (irq_handlers[num].stray_count == -1)
			/* yes. enough is enough, bye */
			return;

		/* increment warn, and get out of here */
		irq_handlers[num].stray_count++;
		
		/* got a high enough stray count? */
		if (irq_handlers[num].stray_count > IRQ_ENOUGH_STRAY) {
			/* yes. log that and stop logging */
			kprintf ("warning: too many stray IRQ 0x%x, not logging anymore\n", num);
			irq_handlers[num].stray_count = -1;
			return;
		}

		/* logging */
		kprintf ("warning: stray IRQ 0x%x\n", num);
		return;
	}

	/* got a variable? */
	if (var != NULL) {
		/* yes. handle it */
		(*var)++;
	} else {
		/* got a thread? */
		if (t != NULL) {
			/* yes. is it already running ? */
			if (t->status & THREAD_STATUS_SUSPENDED) {
				/* no. fire it up */
				thread_resume (t);
			}
		} else {
			/* no. call the handler */
			handler();
		}
	}
}

/*
 * irq_dump()
 *
 * This will perform a dump of all IRQ's allocated.
 *
 */
void
irq_dump() {
	int i;

	/* display them all */
	kprintf ("IRQ dump\n");
	for (i = 0; i <= MAX_IRQS; i++) {
		/* got something here? */
		if ((irq_handlers[i].handler != NULL) ||
			(irq_handlers[i].thread != NULL)) {
			/* yes. display it */
			if (irq_handlers[i].thread != NULL)
				kprintf ("  %u. %s (handler 0x%x, thread 0x%x, var 0x%x\n", i,
									irq_handlers[i].name, irq_handlers[i].thread,
									irq_handlers[i].var);
			else
				kprintf ("  %u. %s (handler 0x%x)\n", i, irq_handlers[i].name,
									irq_handlers[i].handler);
		}
	}
}

/* vim:set ts=2 sw=2: */
