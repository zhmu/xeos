/*
 * cksum.c - XeOS IPv4 checksumming code
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This module handles IPv4 checksums.
 *
 */
#include <sys/types.h>
#include "ipv4.h"

/*
 * ipv4_cksum (char* pkt, int size)
 *
 * This will calculate the IP header checksum of [pkt]
 *
 * From http://www.netfor2.com/ipsum.htm
 *
 */
uint16_t
ipv4_cksum (char* data, int len) {
	uint8_t* b = (uint8_t*)data;
	uint16_t* w = (uint16_t*)data;
	uint32_t sum = 0;
	uint16_t i;
	uint16_t word16;
    
	/* make 16 bit words out of every two adjacent 8 bit words in the packet
	   and add them up */
	for (i=0;i<len;i=i+2){
		word16 =((b[i]<<8)&0xFF00)+(b[i+1]&0xFF);
		sum = sum + (uint32_t) word16;	
	}
	
	/* take only 16 bits out of the 32 bit sum and add up the carries */
	while (sum>>16)
	  sum = (sum & 0xFFFF)+(sum >> 16);

	/* one's complement the result*/
	sum = ~sum;

	/* swap em */
	sum = ((sum & 0xff00) >> 8) | ((sum & 0xff) << 8);
	return sum;
}

/*
 * ipv4_check_ipcksum (IPV4_HEADER* hdr)
 *
 * This will verify the checksum of IPv4 header [hdr]. It will return zero on
 * failure and non-zero on success.
 *
 */
int
ipv4_check_ipcksum (IP_HEADER* hdr) {
	uint16_t old_cksum = hdr->cksum;

	hdr->cksum = 0;
	return (ipv4_cksum ((char*)hdr, sizeof (IP_HEADER)) == old_cksum) ? 1 : 0;
}

/*
 * ipv4_check_icmpcksum (char* pkt, int size)
 *
 * This will verify the checksum of ICMP header [hdr]. It will return zero on
 * failure and non-zero on success.
 *
 */
int
ipv4_check_icmpcksum (ICMP_HEADER* hdr, int len) {
	uint16_t old_cksum = hdr->cksum;

	hdr->cksum = 0;
	return ((ipv4_cksum ((char*)hdr, len) == old_cksum) ? 1 : 0);
}

/* vim:set ts=2 sw=2: */
