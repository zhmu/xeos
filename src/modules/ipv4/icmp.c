/*
 * icmp.c - XeOS IPv4 ICMP Stuff
 *
 * (c) 2003 Rink Springer, BSD
 *
 * This will handle ICMP packets, by RFC 792.
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/mailbox.h>
#include <sys/network.h>
#include "ipv4.h"

#define xICMP_VERBOSE

/*
 * icmp_handle_echorequest (struct NETPACKET* np)
 *
 * This will handle ICMP Echo Request packet [np].
 *
 */
void
icmp_handle_echorequest (struct NETPACKET* np) {
	ICMP_HEADER* ihdr = (ICMP_HEADER*)(np->data + sizeof (IP_HEADER) + 0xe);
	IP_HEADER* iphdr = (IP_HEADER*)(np->data + 0xe);
	char reply[NETWORK_MAX_PACKET_LEN];
	int i, c;

#ifdef ICMP_VERBOSE
	printf ("got ICMP ping request from %u.%u.%u.%u\n", iphdr->source[0], iphdr->source[1], iphdr->source[2], iphdr->source[3]);
#endif /* ICMP_VERBOSE */

	/* copy the original packet over */
	memcpy (reply, np->data, np->len);

	/* MAC address swap */
	for (i = 0; i < 6; i++) {
		reply[i] =     np->data[i + 6];
		reply[i + 6] = np->data[i];
	}

	/* header update */
	iphdr = (IP_HEADER*)(reply + 0xe);

	/* flip the source and dest IP addresses */
	for (i = 0; i < 4; i++) {
		reply[i + 0x1a] = np->data[i + 0x1e];
		reply[i + 0x1e] = np->data[i + 0x1a];
	}

	/* update the IP header checksum */
	iphdr->cksum = 0; c = ipv4_cksum ((char*)iphdr, sizeof (IP_HEADER));
	iphdr->cksum = c;

	/* update the ICMP header */
	ihdr = (ICMP_HEADER*)(reply + 0xe + sizeof (IP_HEADER));
	ihdr->type = ICMP_TYPE_ECHORESPONSE;

	/* update the ICMP checksum */
	ihdr->cksum = 0; c = ipv4_cksum ((char*)ihdr, np->len - 0xe - sizeof (IP_HEADER));
	ihdr->cksum = c;

	/* go */
	network_send_packet (np->device, reply, np->len);
}

/*
 * icmp_handle_packet (struct NETPACKET* np)
 *
 * This will handle ICMP netpacket [np]. It will return 0 on failure or
 * non-zero on success
 *
 */
int
icmp_handle_packet (struct NETPACKET* np) {
	ICMP_HEADER* ihdr = (ICMP_HEADER*)(np->data + sizeof (IP_HEADER) + 0xe);

	/* checksum OK? */
	if (!ipv4_check_icmpcksum (ihdr, np->len - sizeof (IP_HEADER) - 0xe)) {
#ifdef ICMP_VERBOSE
		printf ("icmp packet with bad checksum?\n");
#endif /* ICMP_VERBOSE */
	}

	/* echo packet? */
 	if (ihdr->type == ICMP_TYPE_ECHOREQUEST) {
		/* yes. handle it */
		icmp_handle_echorequest (np);
		return;
	}

	/* discard it */
}

/* vim: set ts=2 sw=2: */
