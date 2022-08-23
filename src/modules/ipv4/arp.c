/*
 * arp.c - XeOS IPv4 ARP Stuff
 *
 * (c) 2003 Rink Springer, BSD
 *
 * This will handle ARP packets, by RFC 826.
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/mailbox.h>
#include <sys/network.h>
#include "ipv4.h"

#define xARP_DEBUG

/*
 * arp_handle_packet (struct NETPACKET* np)
 *
 * This will handle ARP netpacket [np]. It will return 0 on failure or
 * non-zero on success.
 *
 */
int
arp_handle_packet (struct NETPACKET* np) {
	ARP_PACKET* arp = (ARP_PACKET*)(np->data + 0xe);
	int	 opcode = (arp->opcode >> 8) | ((arp->opcode & 0xff) << 8);
	char reply[NETWORK_MAX_PACKET_LEN];
	ARP_PACKET* rarp = (ARP_PACKET*)(reply + 0xe);
	int i;

	if (opcode == ARP_REQUEST) {
#ifdef ARP_DEBUG
		printf ("got ARP request from %u.%u.%u.%u for %u.%u.%u.%u\n",
			         arp->source_addr[0], arp->source_addr[1],
			         arp->source_addr[2], arp->source_addr[3],
			         arp->dest_addr[0], arp->dest_addr[1],
			         arp->dest_addr[2], arp->dest_addr[3]);
#endif /* ARP_DEBUG */

		/* is it for us? */
		if ((arp->dest_addr[0] == ip_addr[0]) && (arp->dest_addr[1] == ip_addr[1]) && (arp->dest_addr[2] == ip_addr[2]) && (arp->dest_addr[3] == ip_addr[3])) {
			/* yes. copy the packet over */
			memcpy (reply, np->data, np->len);

			/* MAC address swap */
			for (i = 0; i < 6; i++) {
				reply[i] =     np->data[i + 6];
				reply[i + 6] = np->data[i];
			}

			/* fill the packet out */
			rarp->opcode = (ARP_REPLY << 8);

			for (i = 0; i < 6; i++)
				rarp->hw_source[i] = ((struct DEVICE_NETDATA*)np->device->data)->hw_addr[i];
		
			/* flip the source and dest IP addresses */
			for (i = 0; i < 4; i++) {
				rarp->source_addr[i] = arp->dest_addr[i];
				rarp->dest_addr[i]   = arp->source_addr[i];
			}

#ifdef ARP_DEBUG
		printf ("sending ARP reply to %u.%u.%u.%u\n",
			         arp->source_addr[0], arp->source_addr[1],
			         arp->source_addr[2], arp->source_addr[3]);
#endif /* ARP_DEBUG */

			/* go */
			network_send_packet (np->device, reply, np->len);
		}
	}
}

/* vim:set ts=2 sw=2: */
