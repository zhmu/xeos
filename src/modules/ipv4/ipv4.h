/*
 * ipv4.h - XeOS IPv4 stack
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 */

/* based on RFC 1700 */
#define IPHDR_PROTO_ICMP	1
#define IPHDR_PROTO_IP		4

/* based on RFC 791 */
typedef struct  __attribute__((packed)) {
	uint8_t			version_ihl;
	uint8_t			tos;
	uint16_t		len;
	uint16_t		id;
	uint16_t		flag_frags;
	uint8_t			ttl;
	uint8_t			proto;
	uint16_t		cksum;
	uint8_t			source[4];
	uint8_t			dest[4];
} IP_HEADER;

/* based on RFC 792 */
typedef struct __attribute__((packed)) {
	uint8_t			type;
	uint8_t			code;
	uint16_t		cksum;
	uint16_t		ident;
	uint16_t		seq;
} ICMP_HEADER;

#define ICMP_TYPE_ECHOREQUEST	8
#define ICMP_TYPE_ECHORESPONSE	0

/* based on RFC 826 */
typedef struct  __attribute__((packed)) {
	uint16_t		hw_type;
	uint16_t		protocol;
	uint8_t			hw_len;
	uint8_t			proto_len;
	uint16_t		opcode;
	uint8_t			hw_source[6];
	uint8_t			source_addr[4];
	uint8_t			hw_dest[6];
	uint8_t			dest_addr[4];
} ARP_PACKET;

#define ARP_HWTYPE_ETH		1

#define ARP_REQUEST				1
#define ARP_REPLY					2

extern char ip_addr[4];

/* vim:set ts=2 sw=2: */
