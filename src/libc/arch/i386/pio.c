/*
 * pio.c - XeOS i386 IO code
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle reading and writing to I/O ports
 *
 */
#include <sys/types.h>

void
outb (unsigned short port, unsigned char data) {
	asm ("out %0,%1"
	:
	: "a" (data), "id" (port));
}

void
outw (unsigned short port, unsigned short data) {
	asm ("outw %0,%1"
	:
	: "a" (data), "d" (port));
}

unsigned char
inb (unsigned short port) {
	unsigned char data;

	asm ("inb %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

unsigned short
inw (unsigned short port) {
	unsigned short data;

	asm ("inw %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

void
insw (int port, void* addr, int cnt) {
   asm ("cld\nrepne\ninsw" :
	"=D" (addr), "=c" (cnt) :
	"d" (port), "0" (addr), "1" (cnt) :
	"memory");
}

/* vim:set ts=2 sw=2: */
