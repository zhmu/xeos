/*
 * init.c - XeOS i386 initialization code
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code is heavily based on Yoctix, most residing in src/arch/i386/, (c)
 * 1999 Anders Gavare.
 *
 * This code will handle setting up the i386-dependant stuff.
 *
 */
#include <md/config.h>
#include <md/gdt.h>
#include <md/init.h>
#include <md/interrupts.h>
#include <md/task.h>
#include <md/vm.h>
#include <lib/lib.h>
#include <sys/kmalloc.h>
#include <sys/irq.h>
#include <sys/thread.h>
#include <config.h>

uint32_t ticks_till_switch;
signed int switch_ratio;

/* declared in startup.c */
extern uint32_t kernelpagedir;

/* declared in syscall_asm.s */
void syscall_asm();

/* forward declarations for the assembly code */
void timer_asm();
void exc0_asm();
void exc1_asm();
void exc2_asm();
void exc3_asm();
void exc4_asm();
void exc5_asm();
void exc6_asm();
void exc7_asm();
void exc8_asm();
void exc9_asm();
void exca_asm();
void excb_asm();
void excc_asm();
void excd_asm();
void exce_asm();
void excf_asm();
void irq0_asm();
void irq1_asm();
void irq2_asm();
void irq3_asm();
void irq4_asm();
void irq5_asm();
void irq6_asm();
void irq7_asm();
void irq8_asm();
void irq9_asm();
void irqa_asm();
void irqb_asm();
void irqc_asm();
void irqd_asm();
void irqe_asm();
void irqf_asm();

/* own declarations */
uint8_t* gdt;
uint8_t* idt;

/*
 * gdt_set_entry (unsigned int no, unsigned int limit, unsigned int addr,
 *		  unsigned int segtype, unsigned int desctype, unsigned int
 *		  dpl, unsigned int pbit, unsigned int opsize, unsigned int
 *		  dbit, unsigned int gran)
 *
 * This will set up GDT entry [no] according to the information supplied.
 *
 */
void
gdt_set_entry (uint8_t no, uint32_t limit, uint32_t addr, uint8_t segtype, uint8_t desctype, uint8_t dpl, uint8_t pbit, uint8_t avl, uint8_t opsize, uint8_t dbit, uint8_t gran) {
	uint8_t* p = (uint8_t*)(gdt + (no * 8));

	/* fill the entry out */
	p[0]  = limit & 0xff;			/* segment limit */
	p[1]  = (limit >> 8) & 0xff;
	p[2]  = addr & 255;				/* base address */
	p[3]  = (addr >> 8) & 255;
	p[4]  = (addr >> 16) & 255;
	p[5]  = segtype;
	p[5] |= (desctype * 16);
	p[5] |= (dpl * 32);
	p[5] |= (pbit * 128);
	p[6]  = ((limit >> 16) & 15);
	p[6] |= (avl * 16);
	p[6] |= (opsize * 32);
	p[6] |= (dbit * 64);
	p[6] |= (gran *  128);
	p[7]  = (addr >> 24) & 255;
}

/* 
 * interrupts_set_entry (uint8_t no, void* handler, uint16_t sel, uint8_t type,
 *                       uint8_t dpl)
 *
 * This will set up the specified interrupt up.
 *
 */
void
interrupts_set_entry (uint8_t no, void* handler, uint16_t sel, uint8_t type, uint8_t dpl) {
	uint8_t* p = (uint8_t*)(idt + (no * 8));

	p[0] = (uint32_t)handler & 0xff;
	p[1] = ((uint32_t)handler >> 8) & 0xff;
	p[2] = sel & 0xff;
	p[3] = (sel >> 8) & 0xff;
	p[4] = 0;
	p[5] = 128 + (dpl * 32) + type;
	p[6] = ((uint32_t)handler >> 16) & 0xff;
	p[7] = ((uint32_t)handler >> 24) & 0xff;
}


/*
 * gdt_init()
 *
 * This will create and activate our new GDT.
 *
 */
void
gdt_init() {
	int sz;
	uint8_t gdt_address[6];

	/* get the size */
	sz = 8 * (4 + MAX_THREADS);

	/* allocate memory */
	gdt = (unsigned char*)kmalloc (NULL, sz, 0);

	/* first of all, clean out the entire GDT, so the NULL descriptor is
	   correct */
	kmemset (gdt, 0, sz);

	/* 1: kernel code */
	gdt_set_entry (1, 0xfffff, 0x0, GDT_SEGTYPE_EXEC | GDT_SEGTYPE_READ_CODE,
	               GDT_DESCTYPE_CODEDATA, 0, 1, 0, 1, 1, 1);

	/* 2: kernel data */
	gdt_set_entry (2, 0xfffff, 0x0, GDT_SEGTYPE_WRITE_DATA,
	               GDT_DESCTYPE_CODEDATA, 0, 1, 0, 1, 1, 1);

	/* 3: user code */
	gdt_set_entry (3, 0xfffff, 0x0, GDT_SEGTYPE_EXEC | GDT_SEGTYPE_READ_CODE,
	               GDT_DESCTYPE_CODEDATA, 3, 1, 0, 1, 1, 1);

	/* 4: user data */
	gdt_set_entry (4, 0xfffff, 0x0, GDT_SEGTYPE_WRITE_DATA,
	               GDT_DESCTYPE_CODEDATA, 3, 1, 0, 1, 1, 1);

	/* use the new GDT */
	gdt_address[0] = (sz - 1) & 0xff;
	gdt_address[1] = (sz - 1) >> 8;
	gdt_address[2] = ((uint32_t)gdt & 0xff);
	gdt_address[3] = ((uint32_t)gdt >>  8) & 0xff;
	gdt_address[4] = ((uint32_t)gdt >> 16) & 0xff;
	gdt_address[5] = ((uint32_t)gdt >> 24) & 0xff;

	/* go! */
	__asm__ ("lgdt (%0)" : : "r" (&gdt_address[0]));

	/* ensure it's all correct now */
	__asm__ ("mov $0x10, %ax");
	__asm__ ("mov %ax, %ds");
	__asm__ ("mov %ax, %es");
	__asm__ ("mov %ax, %ss");
	__asm__ ("mov %ax, %fs");
	__asm__ ("mov %ax, %gs");
	__asm__ ("jmp 1f\n1:\n");
}

/*
 * interrupts_init()
 *
 * This will set up the interrupts.
 *
 */
void
interrupts_init() {
	uint8_t  idt_address[6];

	/* set the PIC up first (this sequence is copied from Yoctix) to remap
	 * interrupts to a sensible location */
	asm ("mov $0x20,%dx\nmov $0x11,%al\nout %al,%dx"); /* outb (0x20, 0x11) */
	asm ("mov $0xa0,%dx\nmov $0x11,%al\nout %al,%dx"); /* outb (0xa0, 0x11) */
	asm ("mov $0x21,%dx\nmov $0x20,%al\nout %al,%dx"); /* outb (0x21, 0x20) */
	asm ("mov $0xa1,%dx\nmov $0x28,%al\nout %al,%dx"); /* outb (0xa1, 0x28) */
	asm ("mov $0x21,%dx\nmov $0x04,%al\nout %al,%dx"); /* outb (0x21, 0x04) */
	asm ("mov $0xa1,%dx\nmov $0x02,%al\nout %al,%dx"); /* outb (0xa1, 0x02) */
	asm ("mov $0x21,%dx\nmov $0x01,%al\nout %al,%dx"); /* outb (0x21, 0x01) */
	asm ("mov $0xa1,%dx\nmov $0x01,%al\nout %al,%dx"); /* outb (0xa1, 0x01) */
	asm ("mov $0x21,%dx\nxor %al,%al\nout %al,%dx");   /* outb (0x21, 0x00) */
	asm ("mov $0xa1,%dx\nxor %al,%al\nout %al,%dx");   /* outb (0xa1, 0x00) */
	
	/* allocate memory for the IDT */
	idt = (unsigned char*)kmalloc (NULL, (8 * 256), 0);

	/* clean out the entire IDT */
	kmemset (idt, 0, (8 * 256));

	/* set the exception handlers */
	interrupts_set_entry (0x0, (void*)exc0_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x1, (void*)exc1_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x2, (void*)exc2_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x3, (void*)exc3_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x4, (void*)exc4_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x5, (void*)exc5_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x6, (void*)exc6_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x7, (void*)exc7_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x8, (void*)exc8_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x9, (void*)exc9_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0xa, (void*)exca_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0xb, (void*)excb_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0xc, (void*)excc_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0xd, (void*)excd_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0xe, (void*)exce_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0xf, (void*)excf_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);

	/* set all irq handlers */
	interrupts_set_entry (0x20, (void*)&irq0_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x21, (void*)&irq1_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x22, (void*)&irq2_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x23, (void*)&irq3_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x24, (void*)&irq4_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x25, (void*)&irq5_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x26, (void*)&irq6_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x27, (void*)&irq7_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x28, (void*)&irq8_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x29, (void*)&irq9_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x2a, (void*)&irqa_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x2b, (void*)&irqb_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x2c, (void*)&irqc_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x2d, (void*)&irqd_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x2e, (void*)&irqe_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);
	interrupts_set_entry (0x2f, (void*)&irqf_asm,
	                      KCODE32_SEL, I386_INT_GATE, 0);

	/* syscall handler */
	interrupts_set_entry (SYSCALL_INT, (void*)&syscall_asm,
	                      KCODE32_SEL, I386_INT_GATE, 3);

	/* build the address */
	idt_address[0] = ((8 * 256) - 1) & 0xff;
	idt_address[1] = ((8 * 256) - 1) >> 8;
	idt_address[2] =  (uint32_t)idt & 0xff;
	idt_address[3] = ((uint32_t)idt >>  8) & 0xff;
	idt_address[4] = ((uint32_t)idt >> 16) & 0xff;
	idt_address[5] = ((uint32_t)idt >> 24) & 0xff;

	/* here goes nothing... */
	__asm__ ("lidt (%0)" : : "r" (&idt_address[0]));
}

/*
 * timer_init()
 *
 * This will handle our timer stuff.
 *
 */
void
timer_init () {
	int v;

	/* program the timer */
	v = 1193180;
	v /= HZ;
	asm ("mov $0x43,%dx\nmov $0x36,%al\nout %al,%dx"); /* outb (0x43, 0x36) */
	asm ("out %0,%1" : : "a" (v & 0xff), "id" (0x40)); /* outb (0x40, v % 0xff) */
	asm ("out %0,%1" : : "a" (v >>   8), "id" (0x40)); /* outb (0x40, v >>   8) */

	/* officially register the IRQ, so no one else can grab it */
	irq_register (0, "timer", (void*)&timer_asm, (struct THREAD*)NULL, 0);

	/* initialize the switch ticks and ratio */
	ticks_till_switch = 1;
	switch_ratio = (HZ / SWITCH_HZ);

	/* the timer is time-critical, so just call it at once, to avoid the IRQ
	   manager bloat */
	interrupts_set_entry (0x20, (void*)&timer_asm, KCODE32_SEL, I386_INT_GATE, 0);
}

/*
 * timer()
 *
 * This is the actual timer interrupt.
 *
 */
void
timer() {
	/* make sure we don't switch too often */
	if (--ticks_till_switch != 0)
		return;
	ticks_till_switch = switch_ratio;

	/* switch! */
	thread_switch ();
}

/*
 * arch_init()
 *
 * This will initialize the machine dependant stuff.
 *
 */
void
arch_init() {
	/* initialize the GDT */
	gdt_init();

	/* initialize the interrupts and exceptions */
	interrupts_init();

	/* initialize the timer */
	timer_init();

	/* initialize threads */
	thread_init();

	/* initialize multitasking */
	arch_task_init();
}

/* vim:set ts=2 sw=2: */
