/*
 * network.c - XeOS Networking Subsystem
 * (c) 2003 Rink Springer, BSD licensed
 *
 * This code will handle networking packet transfers.
 *
 */
#include <kern/syscall.h>
#include <sys/mailbox.h>
#include <sys/thread.h>
#include <sys/network.h>
#include <sys/tty.h>
#include <lib/lib.h>

MAILBOX* mbox_network;

/*
 * network_init()
 *
 * This will initialize the networking subsystem.
 *
 */
void
network_init() {
	/* build a mailbox for the networking system */
	mbox_network = mailbox_create (15, NETWORK_MAX_PACKET_LEN);
}

/*
 * network_add_packet (struct DEVICE* dev, uint8_t* pkt, size_t len)
 *
 * This will add [len] bytes from packet [pkt] on device [dev] to the
 * networking mailbox.
 *
 */
void
network_add_packet (struct DEVICE* dev, uint8_t* pkt, size_t len) {
	struct NETPACKET np;

	/* if the packet is too large, drop it */
	if (len > NETWORK_MAX_PACKET_LEN) {
		kprintf ("network_add_packet(): dropped %u byte dinosaur packet from %x\n", len, dev);
		return;
	}

	/* set the structure up */
	np.device = dev;
	np.len = len;
	kmemcpy (np.data, pkt, len);

	/* add it to the mailbox */
	mailbox_add (mbox_network, (void*)&np, sizeof (struct NETPACKET));
}

/*
 * network_register (struct THREAD* t)
 *
 * This will register thread [t] as network handler.
 *
 */
MAILBOX*
network_register (struct THREAD* t) {
	mailbox_register (mbox_network, t);
	thread_mark_mailbox (t);
	return mbox_network;
}

/*
 * network_send_packet (struct DEVICE* dev, void* pkt, size_t len)
 *
 * This will send [len] bytes of packet [pkt] over the wire of device [dev].
 * This will return zero on failure and non-zero on success.
 *
 */
int
network_send_packet (struct DEVICE* dev, void* pkt, size_t len) {
	return (device_write (dev, 0, pkt, len) == len) ? 1 : 0;
}

/*
 * network_dump()
 *
 * This will dump network information.
 *
 */
void
network_dump() {
	kprintf ("network mailbox: %x in use, %x available\n", mbox_network->numentries, mbox_network->maxentries);
	kprintf ("mailbox thread status: %x\n", mbox_network->wait_thread->status);
}

/* vim:set ts=2 sw=2: */
