/*
 * sys_network.c - XeOS Networking Syscalls
 * (c) 2003 Rink Springer, BSD licensed
 *
 * This code will handle XeOS networking syscalls.
 *
 */
#include <kern/syscall.h>
#include <sys/thread.h>
#include <sys/mailbox.h>
#include <sys/network.h>
#include <sys/types.h>

/*
 * network_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10)
 *
 * This will handle networking syscalls.
 *
 */
uint32_t
network_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10) {
	switch (r1) {
	case SYS_network_add_packet: /* add a packet to the network mailbox */
		                           network_add_packet ((struct DEVICE*)r2, (uint8_t*)r3, (size_t)r4);
															 return 1;
		case SYS_network_register: /* register this thread as mailbox handler */
		                           return (uint32_t)network_register (t);
 case SYS_network_send_packet: /* send a packet over the wire */
															 return network_send_packet ((struct DEVICE*)r2, (void*)r3, (size_t)r4);
	}

	/* ??? */
	return 0;
}

/* vim:set ts=2 sw=2: */
