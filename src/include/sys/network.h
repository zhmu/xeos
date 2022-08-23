/*
 * network.h - XeOS Network Stuff
 * (c) 2003 Rink Springer, BSD
 *
 * This include file describes some stuff for networking.
 *
 */
#ifndef __NETWORK_H__
#define __NETWORK_H__

#define NETWORK_MAX_PACKET_LEN	1600

#include <sys/mailbox.h>

typedef struct  __attribute__((packed)) {
	uint8_t			dest[6];
	uint8_t			source[6];
	uint16_t		type;
} ETHERNET_HEADER;

struct NETPACKET {
	struct DEVICE*	device;
	size_t len;
	char	 data[NETWORK_MAX_PACKET_LEN];
};

#ifdef __KERNEL
void network_init();
void network_dump();
void network_add_packet (struct DEVICE* dev, uint8_t* pkt, size_t len);
MAILBOX* network_register (struct THREAD* t);
int network_send_packet (struct DEVICE* dev, void* pkt, size_t len);
#endif /* __KERNEL */

#endif /* __NETWORK_H__ */

/* vim:set ts=2 sw=2: */
