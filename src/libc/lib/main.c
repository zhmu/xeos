/*
 * main.c - XeOS libc main function
 * (c) 2003 Rink Springer, BSD licensed
 *
 * This code will set stuff up and chain to [main].
 *
 */
#include <sys/types.h>
#include <stdarg.h>

extern void main (void* ptr);
uint32_t ipv4_no;

asm (".GLOBAL libmain");

/*
 * libmain (void* ptr)
 *
 * This is THE main function!
 *
 */
void
libmain (void* ptr) {
	/* scan for syscall numbers for services */
	ipv4_no = find_syscall ("ipv4");
	if (ipv4_no)
		printf ("WE got IPV4!!!!\n");
}

/* vim:set ts=2 sw=2: */
