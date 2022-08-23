/*
 * vm.c - XeOS i386 Paging Code
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle paging.
 *
 */
#include <sys/types.h>
#include <sys/thread.h>
#include <sys/kmalloc.h>
#include <lib/lib.h>
#include <md/config.h>
#include <md/memory.h>
#include <md/vm.h>

#define xPAGING_DEBUG
		
extern uint8_t* kernelpagedir;

/*
 * arch_vm_map (struct THREAD* t, addr_t log, addr_t phys, size_t len)
 *
 * This will map [len] bytes of physical addres [phys] to logical address [log]
 * in thread [t].
 *
 */
int
arch_vm_map (struct THREAD* t, addr_t log, addr_t phys, size_t len) {
	int numpages;
	uint32_t* pagedir = (uint32_t*)(t->tss->cr3 + mapped_kernel);
	uint32_t* page_entry;
	uint16_t pd;
	uint16_t pd_entry;

	/* calculate how much pages to map */
	numpages = (len / PAGESIZE);
	if (len % PAGESIZE) numpages++;

	/* do we have a sensible logical address? */
	if (log & 0xfff)
		/* no! complain */
		panic ("arch_vm_map(): invalid logical address 0x%x\n", log);

	/* do we have a sensible physical address? */
	if (phys & 0xfff)
		/* no! complain! */
		panic ("arch_vm_map(): invalid physical address 0x%x\n", phys);

	/* keep mapping while there are pages */
	while (numpages--) {
		/* determine the page directory indices */
		pd       = (log & 0xffc00000) >> 22;
		pd_entry = (log &   0x3ff000) >> 12;

		/* fetch the entry */
		page_entry = (uint32_t*)(pagedir[pd] & 0xffffff00);

		#ifdef PAGING_DEBUG
		kprintf ("pagedir=%x pd=%x pd_entry=%x page_entry=%x\n", pagedir, pd, pd_entry, page_entry);
		#endif

		/* present page? */
		if (!((uint32_t)page_entry) & 1) {
			/* no. create our own */
			page_entry = (uint32_t*)kmalloc (t, PAGESIZE, 0);
			kmemset (page_entry, 0, PAGESIZE);
#if 0
			if (pagedir[pd] & 1)
				memcpy ((void*)page_entry, (void*)((pagedir[pd] & 0xffffff00) + mapped_kernel), PAGESIZE);
#endif
			pagedir[pd] = ((uint32_t)page_entry - mapped_kernel) | 7; /* 3 */
		} else {
			page_entry = (uint32_t*)((uint32_t)page_entry + (uint32_t)mapped_kernel);
		}

		/* update it */
		page_entry[pd_entry] = (phys) | 7;

		/* next */
		log += PAGESIZE; phys += PAGESIZE;
	}

	return 1;
}

/*
 * arch_vm_setup_thread (struct TSS* t, char* pagedir)
 *
 * This will set up a paging map for TSS [t] in [pagedir].
 *
 */
void
arch_vm_setup_thread (struct TSS* t, char* pagedir) {
	/* got a pagedir? */
	if (pagedir == NULL) {
		/* no. use the kernel's one (this is the kernel TSS then) */
		t->cr3 = (uint32_t)(kernelpagedir - mapped_kernel);
	} else {
		/* yes. copy the kernel pagedir over it and use it */
		kmemcpy (pagedir, (void*)kernelpagedir, PAGESIZE);
		t->cr3 = (uint32_t)(pagedir - mapped_kernel);
	}
}

/*
 * arch_vm_getmap()
 *
 * This will fetch the current page map.
 *
 */
addr_t
arch_vm_getmap() {
	addr_t addr;

	/* just return CR3 */
	asm("movl %%cr3, %%eax": "=a" (addr));
	return addr;
}

/*
 * arch_vm_setmap (addr_t addr)
 *
 * This will change the current page map to [addr].
 *
 *
 */
void
arch_vm_setmap(addr_t addr) {
	/* just change CR3 */
	asm ("movl %%eax, %%cr3": : "a" (addr));

	/* flush the paging cache */
	asm("jmp 1f\n1:");
}

/*
 * arch_vm_setthreadmap(struct THREAD* t)
 *
 * This will change the thread mapping to that of thread [t].
 *
 */
void
arch_vm_setthreadmap(struct THREAD* t) {
	addr_t addr = t->tss->cr3;

	/* activate the thread's CR3 */
	arch_vm_setmap (addr);
}

/* vim:set ts=2 sw=2: */
