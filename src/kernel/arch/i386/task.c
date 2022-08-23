/*
 * thread.c - XeOS i386 thread code
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code is heavily based on Yoctix, most residing in src/arch/i386/, (c)
 * 1999 Anders Gavare.
 *
 * This code will handle setting up the thread i386-dependant stuff.
 *
 */
#include <sys/types.h>
#include <sys/kmalloc.h>
#include <sys/thread.h>
#include <lib/lib.h>
#include <md/config.h>
#include <md/gdt.h>
#include <md/task.h>
#include <md/vm.h>

struct TSS kernel_tss;

/*
 * arch_thread_launch (struct THREAD* t);
 *
 * This will activate thread [t].
 *
 */
void
arch_thread_launch (struct THREAD* t) {
	uint8_t buf[6];
	uint16_t nr = t->tss_no;

	nr += TSS_SEL; nr *= 8; 
	buf[0] = 0; buf[1] = 0; buf[2] = 0; buf[3] = 0;
	buf[4] = (nr & 0xff); buf[5] = (nr >> 8);

	__asm__ ("ljmp *(%%eax)" : : "a" (&buf));
}

/*
 * task_create_tss (uint16_t no, struct THREAD* t, char* pagedir)
 *
 * This will create task [no] with tss [tss] and pagedirectory [pagedir].
 *
 */
void
task_create_tss (uint16_t no, struct TSS* tss, char* pagedir) {
	size_t len;

	/* length is tss + iomap */
	if (pagedir)
		len = sizeof (struct TSS) + 8192;
	else
		len = sizeof (struct TSS);

	/* build the TSS GDT entry */
	kmemset ((void*)tss, 0, len);
	gdt_set_entry (no, len, (addr_t)tss, GDT_SEGTYPE_AVAILABLETSS,
	               GDT_DESCTYPE_SYSTEM, 0, 1, 0, 1, 1, 0);

	/* set the pagedir up */
	arch_vm_setup_thread (tss, pagedir);
}

/*
 * task_init_user_tss (struct TSS* tss, void* eip)
 *
 * This will initialize tss [tss] for an user task.
 *
 */
void
task_init_user_tss (struct TSS* tss, void* eip) {
	tss->eflags = /*0x202*/ TSS_EFLAGS_IF | TSS_EFLAGS_RF1;
	tss->iobase = sizeof (struct TSS) - 4;

	/* set the segments up */
	tss->cs = UCODE32_SEL + 3;
	tss->ds = UDATA32_SEL;
	tss->es = UDATA32_SEL;
	tss->fs = UDATA32_SEL;
	tss->gs = UDATA32_SEL;
	tss->esp = 0;
	tss->eip = (uint32_t)eip;

	/* close the entire io bitmap down */
	kmemset (&tss->iobitmap, 0xFF, 8192);
}

/*
 * arch_task_init()
 *
 * This will initialize our multitasking.
 *
 */
void
arch_task_init() {
	uint16_t nr = (KERNEL_TSS_SEL * 8);

	kmemset (&kernel_tss, 0, sizeof (struct TSS));
	task_create_tss (KERNEL_TSS_SEL, &kernel_tss, NULL);
	asm ("ltr %%ax" : : "a" (nr));
}


/*
 * arch_thread_init (uint32_t taskno, struct THREAD* t)
 *
 * This will initialize thread [t].
 *
 */
void
arch_thread_init (uint32_t taskno, struct THREAD* t) {
	uint8_t* pagedir;

	/* allocate a TSS */
	t->tss = (struct TSS*)kmalloc (NULL, sizeof (struct TSS) + 8192, 0);
	t->tss_no = taskno;

	/* allocate a page directory */
	pagedir = (uint8_t*)kmalloc (t, PAGESIZE, 0);

	/* initialize the task's TSS */
	task_create_tss (TSS_SEL + t->tss_no, t->tss, pagedir);
	task_init_user_tss (t->tss, NULL);

	/* initialize the stack */
	t->stack =  (char*)kmalloc (NULL, PSTACK, 0);
	t->kstack = (char*)kmalloc (NULL, KSTACK, 0);

	/* set the page directory up */
	arch_vm_setup_thread (t->tss, (char*)pagedir);

	t->tss->ss = UDATA32_SEL + 3;
	t->tss->ss0 = KDATA32_SEL;
	t->tss->esp = (uint32_t)(t->stack + PSTACK);
	t->tss->esp0 = (uint32_t)(t->kstack + KSTACK);
}

/*
 * arch_thread_seteip (struct THREAD* t, addr_t eip)
 *
 * This will set the instruction pointer (program counter) of their [t] to
 * [eip].
 *
 */
void
arch_thread_seteip (struct THREAD* t, addr_t eip) {
	t->tss->eip = eip;
}

/*
 * arch_thread_cleanup (struct THREAD* t)
 *
 * This will clean thread [t] up.
 *
 */
void
arch_thread_cleanup (struct THREAD* t) {
	/* zap the TSS of the thread */
	kfree (t->tss);

	/* free the gdt entry */
	gdt_set_entry (TSS_SEL + t->tss_no, 0, (addr_t)NULL, 0,
	               0, 0, 0, 0, 0, 0, 0);
}

/*
 * arch_thread_pio_enable (struct THREAD* t, addr_t start, size_t len)
 *
 * This will activate [len] IO ports for thread [t], starting at [start].
 *
 */
void
arch_thread_pio_enable (struct THREAD* t, addr_t start, size_t len) {
	uint8_t* iobitmap = (uint8_t*)&t->tss->iobitmap;
	addr_t i, offs;
	uint8_t mask;

	/* enable the ports */
	for (i = start; i <= start + len; i++) {
		/* calculate the bitmask and start byte */
		offs = (i >> 3); mask = (1 << (i & 7));
		iobitmap[offs] &= ~mask;
	}
}

/*
 * arch_thread_push (struct THREAD* t,uint32_t v)
 *
 * This will push [v] on thread [t]'s stack.
 *
 */
void
arch_thread_push (struct THREAD* t,uint32_t v) {
	uint32_t* ptr = (uint32_t*)(t->tss->esp);

	*ptr = v;
	t->tss->esp -= 4;
}

/* vim:set ts=2 sw=2: */
