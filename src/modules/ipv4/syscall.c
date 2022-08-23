/*
 * syscall.c - XeOS IPv4 Syscalls
 *
 * (c) 2003 Rink Springer, BSD
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/mailbox.h>
#include <sys/network.h>
#include "ipv4.h"

uint32_t ipv4_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10) {
	printf ("!!");
	return 0;
}

/* vim:set ts=2 sw=2: */
