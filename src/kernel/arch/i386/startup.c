/*
 * startup.c - XeOS i386 Startup code
 * (c) 2002 Rink Springer, BSD licensed
 *
 * This code is heavily based on YCX2 (src/ycx2/arch/i386/jumpmain.c), (c) 2001,
 * 2002 Anders Gavare.
 *
 * This code will create a paging table, so references are not messed up.
 *
 */
#include <sys/types.h>
#include <md/config.h>
#include <md/memory.h>

/* forward declaration to kernel main */
void kmain();

/* our own stuff */
size_t	mapped_kernel = 0;
size_t	highest_addressable_byte = 0;
size_t	first_addressable_high_byte = 0;
size_t	lowest_addressable_free_byte = 0;
size_t	kernelpagedir = 0;
size_t	kernelpagedir_raw = 0;

/*
 * i386_trymemory (uint8_t* ptr, size_t len)
 *
 * This will scan [ptr] for 0xff bytes, to probe whether memory exists or not.
 * It will return 0 if [ptr] only has 0xff bytes, or 1 if it does not.
 *
 */
int
i386_trymemory (uint8_t* ptr, size_t len) {
	size_t i;

	/* scan the range */
	for (i = 0; i < len; i++)
		/* 0xff byte here? */
		if (ptr[i] != 0xff)
			return 1;

	/* only 0xff bytes here */
	return 0;
}

/*
 * i386_getkernelsize()
 *
 * This returns the size of the kernel.
 *
 */
size_t
i386_getkernelsize() {
	/* FIXME: how to calculate ELF sizes ??? */
	return 300000;
}

/*
 * i386_getkernelloadaddr()
 *
 * This will return where the kernel is loaded.
 *
 * FIXME: For now, this function only guesses...
 *
 */
size_t i386_getkernelloadaddr() {
	int addr;

	addr = (int) &i386_getkernelloadaddr;
	addr &= 0x00ffffff;

	if (addr >= 0x100000)
		return 0x100000;
	else
		return 0x10000;
}

/*
 * __main()
 *
 * This will set up paging.
 *
 */
void
__main() {
	int i, j;
	size_t *mapped_kernel_ptr;
	size_t highest, try;
	ssize_t trylen;
	size_t top_of_memory, bottom_of_high_memory;
	int nr_of_pagetables;
	int kernel_image_size;
	size_t low_memory_usage = 0;
	size_t high_memory_usage = 0;
	unsigned int *pagedir;
	unsigned int *pagetable;

	/* find out where the kernel should be mapped. that is, where the kernel
	 * itself thinks it belongs */
	mapped_kernel_ptr = (size_t *)((size_t)(&mapped_kernel) & 0x00ffffff);
	*mapped_kernel_ptr = ((size_t)(&__main) & 0xff000000);

	/* now, find the last byte of memory we can use */
	/* TODO: Anders said this code sucked, and it could use some touching-up */
	try = 0x100000;
	trylen = 0x100000;      /*  step size  */
	highest = try;

	/* keep scanning until it fails */
	while (1) {
		if (!i386_trymemory ((uint8_t*)try, trylen))
			/* this failed. bail out */
			break;

		/* more memory... quick! me hungry! */
		try += trylen;
	}
	highest = try - 1;
	top_of_memory = highest;

	/* calculate the number of pagetables we need. each pagetable covers 4MB,
	 * so it's just division really */
	highest |= 0x3fffff;
	nr_of_pagetables = ((size_t)highest+1) / (4096*1024);

	/*  Estimate the size of the kernel image in memory:  */
	kernel_image_size = i386_getkernelsize();
	kernel_image_size = ((kernel_image_size - 1) | (PAGESIZE-1)) + 1;

	if (i386_getkernelloadaddr() < 1048576)
		low_memory_usage += kernel_image_size;
	else
		high_memory_usage += kernel_image_size;

	/*
	 *  Place the kernel pagedirectory just right after the kernel
	 *  image in memory, and the pagetables right after that.
	 */

	pagedir = (uint32_t *) (i386_getkernelloadaddr() + kernel_image_size);

	/* build the page directory */
	for (i=0; i<1024; i++) {
		/* need to build this page table? */
		if (i < nr_of_pagetables) {
			/* yes. place it right after the directory itself */
			pagetable = (uint32_t *)(((void *)pagedir) + (PAGESIZE * (i + 1)));
			pagedir[i] = (uint32_t)pagetable + 7/*3*/;

			/*  TODO:  the value 3 is of course bits for
				"read/write" and "present"... these should
				be changed to #defined names...!!!!!!  */
			for (j=0; j<1024; j++) {
				/* create the correct reference */
				pagetable[j] = 7 /*3*/ + (i*1024+j)*PAGESIZE;
			}
		} else
			/* no such page */
			pagedir[i] = 0;
	}

	/*  Duplicate (map) all low memory to high memory:  */
	j = (*mapped_kernel_ptr) >> 22;
	for (i = 0; i < nr_of_pagetables; i++)
		pagedir[i + j] = pagedir[i];

	if (i386_getkernelloadaddr() < 1048576)
		low_memory_usage += ((1+nr_of_pagetables)*PAGESIZE);
	else
		high_memory_usage += ((1+nr_of_pagetables)*PAGESIZE);

	bottom_of_high_memory = 0x100000 + high_memory_usage;

	/* enable paging ... */
	asm("movl %%eax, %%cr3": : "a" (pagedir));
	asm("jmp 1f\n1:");
	asm("movl %cr0, %eax");
	asm("orl $0x80010033, %eax");
	asm("movl %eax, %cr0");
	asm("jmp 2f\n2:");

	/*  Jump to high memory  (sets eip):  */
	asm("movl $yooo, %eax");
	asm("pushl %eax");
	asm("ret");
	asm("yooo:");

	/*  Fix esp and ebp too, otherwise we'd crash very soon:  */
	asm("addl %%eax, %%esp": : "a" ((*mapped_kernel_ptr)));
	asm("addl %%eax, %%ebp": : "a" ((*mapped_kernel_ptr)));

	/*  Remove everything mapped at low addresses:  */
	for (i = 0; i < nr_of_pagetables; i++)
		pagedir[i] = 0;

	/*  Use the final pagedirectory:  */
	asm("movl %%eax, %%cr3": : "a" (pagedir));
	asm("jmp 1f\n1:");

	/*
	 *  NOTE:  From this point, the only accessible memory is
	 *         from 0x80000000 and up    (or whatever mapped_kernel
	 *         is set to).  Any reference to low memory, for example
	 *	   0x10000, will cause a page fault exception!
	 */

	highest_addressable_byte = top_of_memory;
	first_addressable_high_byte = bottom_of_high_memory;
	kernelpagedir_raw = (size_t) pagedir;
	kernelpagedir = (size_t) pagedir + mapped_kernel;

	/*  0x10000 = above initial stack  */
	lowest_addressable_free_byte = 0x10000 + low_memory_usage;

	/* go to the main code */
	kmain();
}

/* vim:set sw=2 ts=2: */
