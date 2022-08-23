/*
 * ip.c - XeOS IPv4 IP Stuff
 *
 * (c) 2003 Rink Springer, BSD
 *
 * This will handle IP packets, by RFC 791.
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/mailbox.h>
#include <sys/network.h>
#include "ipv4.h"

#define xIP_VERBOSE

/*
 * ip_handle_packet (struct NETPACKET* np)
 *
 * This will handle IP netpacket [np]. It will return 0 on failure or
 * non-zero on success
 *
 */
int
ip_handle_packet (struct NETPACKET* np) {
	IP_HEADER* iphdr = (IP_HEADER*)(np->data + 0xe);

	/* IPv4 thing? */
	if ((iphdr->version_ihl >> 4) != 4)
		/* no. not our cup of tea */
		return;

	/* do we have a valid header? */
	if (!ipv4_check_ipcksum (iphdr)) {
		/* no. complain */
#ifdef IP_VERBOSE
		printf ("dropping packet with bad IP header checksum\n");
#endif /* IP_VERBOSE */
		return 0;
	}

	/* ICMP thing? */
	if (iphdr->proto == IPHDR_PROTO_ICMP) {
		/* yes. have ICMP handle it */
		icmp_handle_packet (np);
		return 1;
	}

	/* discard it */
	return 1;
}

/* vim: set ts=2 sw=2: */
