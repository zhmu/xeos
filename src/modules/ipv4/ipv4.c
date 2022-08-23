/*
 * ipv4.c - XeOS IPv4 Driver
 *
 * (c) 2003 Rink Springer, BSD
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/mailbox.h>
#include <sys/network.h>
#include "ipv4.h"

uint32_t ipv4_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10);

char ip_addr[4] = { 10, 0, 1, 5 };

/*
 * do_packet (char* packet, size_t len)
 *
 * This will actually handle packets.
 *
 */
void
do_packet (struct NETPACKET* np, size_t len) {
	/* try IP first */
	if (!ip_handle_packet (np)) return;

	/* do ARP */
	if (!arp_handle_packet (np)) return;

	/* ??? */
}

int
main() {
	size_t len;
	MAILBOX* mbox;
	struct NETPACKET np;

	/* register us as handler for the network mailbox */
	mbox = network_register();

	/* register our syscall, too */
	if (!register_syscall ("ipv4", &ipv4_syscall)) {
		printf ("IPv4: unable to register syscall, exiting\n");
		exit (1);
	}

	while (1) {
		/* zzzz */
		thread_suspend();

		/* grab the packet */
		while (mbox_fetch (mbox, &np, &len))
			/* handle it */
			do_packet (&np, np.len);
	}
}

/* vim:set ts=2 sw=2: */
