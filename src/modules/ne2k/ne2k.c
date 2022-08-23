/*
 * ne2k.c - XeOS Ne2000 Card Driver
 * (c) 2002 Rink Springer, BSD licensed
 *
 * This code is based on OpenBSD's sys/dev/isa/if_ed.c file, (c) 1994, 1995
 * Charles M. Hannum, (c) 1993 David Greenman.
 *
 * This module handles all NE2000 device stuff.
 *
 */
#include <sys/device.h>
#include <sys/mailbox.h>
#include <sys/network.h>
#include <sys/types.h>
#include <stdlib.h>
#include "ne2k.h"
#include "ne2k_reg.h"

NE2K_CONFIG edata;

/* delay() is a dirty hack, FIXME! */
void delay (unsigned int i) { while (i--); }

/*
 * ne2k_pio_readmem (MODULECONFIG* cf, NE2K_CONFIG* nc, uint16_t src, uint8_t* dst,
 *                 uint16_t size)
 *
 * This will copy memory from the NIC to the buffer
 *
 */
void
ne2k_pio_readmem (MODULECONFIG* cf, NE2K_CONFIG* nc, uint16_t src, uint8_t* dst, uint16_t size) {
	int i;
	uint16_t c;

	/* select page 0 registers */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_0 | NE2K_CR_STA);

	/* round up to a word */
	if (size & 1)
		++size;

	/* set up dma byte count */
	outb (cf->resources->port + NE2K_P0_RBCR0, size);
	outb (cf->resources->port + NE2K_P0_RBCR1, size >> 8);

	/* set up source address in the nic memory */
	outb (cf->resources->port + NE2K_P0_RSAR0, src);
	outb (cf->resources->port + NE2K_P0_RSAR1, src >> 8);
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD0 | NE2K_CR_PAGE_0 | NE2K_CR_STA);

	/* off we go */
	if (nc->isa16bit) {
		/* 16 bit */
		for (i = 0; i < (size >> 1); i++) {
			c = inw (cf->resources->port + 0x10);
			*(uint16_t*)dst = c;
			dst += 2;
		}
	} else {
		/* 8 bit */
		for (i = 0; i < size; i++) {
			*dst = inb (cf->resources->port + 0x10);
			dst++;
		}
	}
}

/*
 * ne2k_pio_writemem (MODULECONFIG* cf, NE2K_CONFIG* nc, uint8_t* src, uint16_t dst,
 *                  uint16_t size)
 *
 * This will copy memory from the buffer to the NIC. [size] must be even!
 *
 */
void
ne2k_pio_writemem (MODULECONFIG* cf, NE2K_CONFIG* nc, uint8_t* src, uint16_t dst, uint16_t size) {
	uint16_t c;
	int maxwait = 100;
	int i;

	/* select page 0 registers */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_0 | NE2K_CR_STA);

	/* reset remote dma complete flag */
	outb (cf->resources->port + NE2K_P0_ISR, NE2K_ISR_RDC);

	/* set up dma byte count */
	outb (cf->resources->port + NE2K_P0_RBCR0, size);
	outb (cf->resources->port + NE2K_P0_RBCR1, size >> 8);

	/* set up destination address in nic memory */
	outb (cf->resources->port + NE2K_P0_RSAR0, dst);
	outb (cf->resources->port + NE2K_P0_RSAR1, dst >> 8);

	/* set remote DMA write */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD1 | NE2K_CR_PAGE_0 | NE2K_CR_STA);

	/* 16 bit? */
	if (nc->isa16bit) {
		/* yes. do it */
		for (i = 0; i < (size >> 1); i++) {
			c = (*src) | (*(src + 1) << 8);
			outw (cf->resources->port + 0x10, c);
			src += 2;
		}
	} else {
		/* no. 8 bit it is then */
		for (i = 0; i < size; i++) {
			outb (cf->resources->port + 0x10, *src);
			src++;
		}
	}

	/* wait for remote DMA completion */
	while (((inb (cf->resources->port + NE2K_P0_ISR) & NE2K_ISR_RDC) != NE2K_ISR_RDC) && (--maxwait));
}

/*
 * modne2k_probe(MODULECONFIG* cf, NE2K_CONFIG* nc)
 *
 * This will probe for a NE2000 card at offset [cf]. It will return 0 on
 * success or -1 on failure.
 *
 */
int
modne2k_probe(MODULECONFIG* cf, NE2K_CONFIG* nc) {
	uint8_t i;
	static uint8_t test_pattern[32] = "THIS is A memory TEST pattern";
	uint8_t test_buffer[32];
	uint8_t	romdata[16];
	int n;

	/* reset the card */
	i = inb (cf->resources->port + NE2K_PORT_RESET);
	outb (cf->resources->port + NE2K_PORT_RESET, i);
	delay (5000);

	/* ensure the nic is correctly reset */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_0 | NE2K_CR_STP);
	delay (5000);

	/* try to probe this card as an generic 8390 */
	if ((inb (cf->resources->port + NE2K_P0_CR) & (NE2K_CR_RD2 | NE2K_CR_TXP | NE2K_CR_STA | NE2K_CR_STP)) != (NE2K_CR_RD2 | NE2K_CR_STP))
		return -1;
	if ((inb (cf->resources->port + NE2K_P0_ISR) & NE2K_ISR_RST) != NE2K_ISR_RST)
		return -1;

	/* Now, we must see if this is a NE1000 or NE2000. This can be done by
		 reading/writing the memory */

	/* turn packet storage in the NIC's memory off */
	outb (cf->resources->port + NE2K_P0_RCR, NE2K_RCR_MON);

	/* initialize the DCR for byte operations */
	outb (cf->resources->port + NE2K_P0_DCR, NE2K_DCR_FT1 | NE2K_DCR_LS);
	outb (cf->resources->port + NE2K_P0_PSTART, 8192 >> NE2K_PAGE_SHIFT);
	outb (cf->resources->port + NE2K_P0_PSTOP, 16384 >> NE2K_PAGE_SHIFT);

	/* 8 bit mode */
	nc->isa16bit = 0;

	/* write a test pattern in byte mode. if this fails, then there is probably
		 no memory at 8k - which makes it likely this board is a NE2000 */
	ne2k_pio_writemem (cf, nc, test_pattern, 8192, sizeof (test_pattern));
	ne2k_pio_readmem  (cf, nc, 8192, test_buffer,  sizeof (test_buffer));

	/* match? */
	if (!memcmp (test_pattern, test_buffer, sizeof (test_pattern))) {
		/* yes. too bad, NE1000 is too ancient for us [XXX] */
		printf ("%s: sorry, but NE1000 is unsupported for now\n", cf->name);
		return -1;
	}

	/* not a ne1000. try ne2000 */
	outb (cf->resources->port + NE2K_P0_DCR, NE2K_DCR_WTS | NE2K_DCR_FT1 | NE2K_DCR_LS);
	outb (cf->resources->port + NE2K_P0_PSTART, 16384 >> NE2K_PAGE_SHIFT);
	outb (cf->resources->port + NE2K_P0_PSTOP, 32768 >> NE2K_PAGE_SHIFT);

	/* 16 bit mode */
	nc->isa16bit = 1;

	/* try to write the test pattern in word mode. if this fails, there is not
		 a ne2000 */
	ne2k_pio_writemem (cf, nc, test_pattern, 16384, sizeof (test_pattern));
	ne2k_pio_readmem  (cf, nc, 16384, test_buffer,  sizeof (test_buffer));

	/* match? */
	if (memcmp (test_pattern, test_buffer, sizeof (test_pattern))) {
		/* no. what's this? */
		printf ("%s: not NE1000, not NE2000 ... what's this ??\n", cf->name);
		return -1;
	}

	/* total of 16kb memory */
	nc->mem_size = 16384;

	/* calculate the offsets */
	nc->mem_start = 16384;
	nc->tx_page_start = nc->mem_size >> NE2K_PAGE_SHIFT;
	nc->mem_end = nc->mem_start + nc->mem_size;
	nc->txb_cnt = 2;

	nc->rec_page_start = nc->tx_page_start + nc->txb_cnt * NE2K_TXBUF_SIZE;
	nc->rec_page_stop = nc->tx_page_start + (nc->mem_size >> NE2K_PAGE_SHIFT);

	nc->mem_ring = nc->mem_start + ((nc->txb_cnt * NE2K_TXBUF_SIZE) << NE2K_PAGE_SHIFT);

	ne2k_pio_readmem (cf, nc, 0, romdata, 16);
	for (n = 0; n < 6; n++)
		nc->addr[n] = romdata[n * 2];

	/* remove any pending irq's */
	outb (cf->resources->port + NE2K_P0_ISR, 0xff);

	/* victory */
	return 0;
}

/*
 * ne2k_init (MODULECONFIG* cf, NE2K_CONFIG* nc)
 *
 * This will initialize initialize the card.
 */
void
ne2k_init (MODULECONFIG* cf, NE2K_CONFIG* nc) {
	int i;

	/* reset flags */
	nc->txb_inuse = 0;
	nc->txb_new = 0;
	nc->txb_next_tx = 0;

	/* set interface for page 0, remote dma complete, stopped */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_0 | NE2K_CR_STP);

	if (nc->isa16bit) {
		/* set fifo treshold to 8. no auto-init remote da, byte order x86, word
			 wide dma xfers */
		outb (cf->resources->port + NE2K_P0_DCR, NE2K_DCR_FT1 | NE2K_DCR_WTS | NE2K_DCR_LS);
	} else {
		/* same as above, but byte-wide DMA xfers */
		outb (cf->resources->port + NE2K_P0_DCR, NE2K_DCR_FT1 | NE2K_DCR_LS);
	}

	/* clear remote byte count registers */
	outb (cf->resources->port + NE2K_P0_RBCR0, 0);
	outb (cf->resources->port + NE2K_P0_RBCR1, 0);

	/* tell RCR to do nothing for now */
	outb (cf->resources->port + NE2K_P0_RCR, NE2K_RCR_MON);

	/* place nic in internal loopback mode */
	outb (cf->resources->port + NE2K_P0_TCR, NE2K_TCR_LB0);

	/* initialize receive buffer ring */
	outb (cf->resources->port + NE2K_P0_BNRY, nc->rec_page_start);
	outb (cf->resources->port + NE2K_P0_PSTART, nc->rec_page_start);
	outb (cf->resources->port + NE2K_P0_PSTOP, nc->rec_page_stop);

	/* clear all interrupts */
	outb (cf->resources->port + NE2K_P0_ISR, 0xff);

	/* enable the following interrupts: recv/xmit complete, recx/xmit error,
	 *																	recv overwrite
	 */
	outb (cf->resources->port + NE2K_P0_IMR,
			NE2K_IMR_PRXE | NE2K_IMR_PTXE | NE2K_IMR_RXEE | NE2K_IMR_TXEE |
      NE2K_IMR_OVWE);

	/* program command register for page 1 */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_1 | NE2K_CR_STP);

	/* copy out our station address */
	for (i = 0; i < 6; i++)
		outb (cf->resources->port + NE2K_P1_PAR0 + i, nc->addr[i]);

	/* TODO: multicast? */

	/* set current page pointer to one page after the boundary pointer */
	nc->next_packet = nc->rec_page_start + 1;
	outb (cf->resources->port + NE2K_P1_CURR, nc->next_packet);

	/* program command registers for page 0 */
	outb (cf->resources->port + NE2K_P1_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_0 | NE2K_CR_STP);

	/* allow broadcasts */
	outb (cf->resources->port + NE2K_P0_RCR, NE2K_RCR_AB);

	/* get rid of the loopback mode */
	outb (cf->resources->port + NE2K_P0_TCR, 0);

	/* fire up the interface */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_0 | NE2K_CR_STA);
}

/*
 * ne2k_stop (MODULECONFIG* cf, NE2K_CONFIG* nc)
 *
 * This will stop the interface.
 *
 */
void
ne2k_stop (MODULECONFIG* cf, NE2K_CONFIG* nc) {
	int n = 5000;

	/* stop the entire interface and select page 0 registers */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_0 | NE2K_CR_STP);

	/* wait for the interface to reach stopped state */
	while (((inb (cf->resources->port + NE2K_P0_ISR) & NE2K_ISR_RST) == 0) && --n);
}

/*
 * ne2k_reset (MODULECONFIG* cf, NE2K_CONFIG* nc)
 *
 * This will reset the interface.
 *
 */
void
ne2k_reset (MODULECONFIG* cf, NE2K_CONFIG* nc) {
	ne2k_stop (cf, nc);
	ne2k_init (cf, nc);
}

/*
 * ne2k_ring_copy (MODULECONFIG* cf, NE2K_CONFIG* nc, int src, char* dst, int len)
 *
 * This will copy a packet from the NIC to a pointer.
 *
 */
int
ne2k_ring_copy (MODULECONFIG* cf, NE2K_CONFIG* nc, uint16_t src, uint8_t* dst, uint16_t len) {
	int tmp_amount;

	/* does copy wrap to lower addr in ring buffer? */
	if (src + len > nc->mem_end) {
		/* yes. handle it */
		tmp_amount = nc->mem_end - src;
		ne2k_pio_readmem (cf, nc, src, dst, tmp_amount);
		len -= tmp_amount;
		src = nc->mem_ring;
		dst += tmp_amount;
	}

	ne2k_pio_readmem (cf, nc, src, dst, len);
	return src + len;
}

/*
 * ne2k_get (MODULECONFIG* cf, NE2K_CONFIG* nc, int src, int len, char* ptr)
 *
 * This will fetch the packet into [ptr]. It will return 0 on failure or 1
 * on success.
 *
 */
int
ne2k_get (MODULECONFIG* cf, NE2K_CONFIG* nc, uint16_t src, uint16_t len, uint8_t* ptr) {
	src = ne2k_ring_copy (cf, nc, src, ptr, len);
	return 1;
}

/*
 * ne2k_read (MODULECONFIG* cf, NE2K_CONFIG* nc, int buf, int len)
 *
 * This will retrieve the packet.
 *
 */
void
ne2k_read (MODULECONFIG* cf, NE2K_CONFIG* nc, int buf, int len) {
	uint8_t ptr[2048];

	/* fetch the packet from the card */
	if (!ne2k_get (cf, nc, buf, len, ptr)) {
		/* this failed, return */
		return;
	}

#if 0
	printf ("%s: got packet of %u bytes\n", cf->name, len);
	for (i = 0xe; i < len; i++)
		printf ("[%x] ", ptr[i]);
	printf ("\n");

	/* IPv4 thing? */
	if ((iphdr->version_ihl >> 4) == 4) {
		/* yes! header ok? */
		if (!ne2k_ip_cksum (iphdr)) {
			/* no! */
			printf ("%s: IP header checksum error, dropping packet!\n", cf->name);
			return;
		}

		/* ping? */
 		if (icmphdr->type == 8) {
			printf ("%s: got ICMP ping request from %u.%u.%u.%u\n", cf->name, iphdr->source[0], iphdr->source[1], iphdr->source[2], iphdr->source[3]);
			if (!ne2k_icmp_cksum (icmphdr, len - 0xe)) {
				printf ("%s: ICMP header checksum error!\n", cf->name);
			}

			/* MAC address swap */
			for (i = 0; i < 6; i++) {
				res[i] =     ptr[i + 6];
				res[i + 6] = ptr[i];
			}
			/* header update */
			iphdr = (IP_HEADER*)(res + 0xe);
			/* IP swap */
			for (i = 0; i < 4; i++) {
				res[i + 0x1a] = ptr[i + 0x1e];
				res[i + 0x1e] = ptr[i + 0x1a];
			}
			/* checksum update */
			iphdr->cksum = 0;
			c = ne2k_cksum ((char*)iphdr, sizeof (IP_HEADER));
			iphdr->cksum = c;
			if (!ne2k_ip_cksum (iphdr)) {
				printf ("%s: fucked up our own IP checksum\n", cf->name);
			}
			iphdr->cksum = c;
			icmphdr = (ICMP_HEADER*)(res + 0xe + sizeof (IP_HEADER));
			icmphdr->type = 0;
			icmphdr->cksum = 0;
			c = ne2k_cksum ((char*)icmphdr, len - 0xe);
			icmphdr->cksum = c;
			if (!ne2k_icmp_cksum (icmphdr, len - 0xe)) {
				printf ("%s: fucked up our own ICMP checksum\n", cf->name);
			}
			icmphdr->cksum = c;
/*
			for (i = 0xe; i < len; i++)
				printf ("[%x] ", res[i]);
			printf ("\n");
			*/
			ne2k_start (cf, nc, res, len + 2);
		}
	} else {
		/* arp packet maybe? */
		int opcode = (arp->opcode >> 8) | ((arp->opcode & 0xff) << 8);
		if (opcode == ARP_REQUEST) {
			printf ("%s: got ARP request from %u.%u.%u.%u for %u.%u.%u.%u\n",
			         cf->name,
			         arp->source_addr[0], arp->source_addr[1],
			         arp->source_addr[2], arp->source_addr[3],
			         arp->dest_addr[0], arp->dest_addr[1],
			         arp->dest_addr[2], arp->dest_addr[3]);

			if (arp->dest_addr[3] == 5) { 
				/* build the reply */
				/* MAC address swap */
				for (i = 0; i < 6; i++) {
					res[i] =     ptr[i + 6];
					res[i + 6] = ptr[i];
				}
				/* fill the packet out */
				narp->opcode = (ARP_REPLY << 8);

				for (i = 0; i < 6; i++)
					narp->hw_source[i] = nc->addr[i];

				/* HACK */
				narp->source_addr[0] = 0xa;
				narp->source_addr[1] = 0;
				narp->source_addr[2] = 1;
				narp->source_addr[3] = 5;

				for (i = 0; i < 4; i++)
					narp->dest_addr[i] = arp->source_addr[i];

				/* send it */
				ne2k_start (cf, nc, res, len + 2);
			}
		}
	}
#endif

	/* put it in the kernel mailbox! */
	network_add_packet (nc->device, ptr, len);
}

/*
 * ne2k_rint (MODULECONFIG* cf, NE2K_CONFIG* nc)
 *
 * This will handle receive interrupts.
 *
 */
void
ne2k_rint (MODULECONFIG* cf, NE2K_CONFIG* nc) {
	uint8_t boundary, current;
	uint16_t len;
	uint8_t nlen;
	uint8_t next_packet;
	uint16_t count;
	uint8_t packet_hdr[NE2K_RING_HDRSZ];
	int packet_ptr;

loop:
	/* set nic to page 1 registers to get 'current' pointer */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_1 | NE2K_CR_STA);

	/* get the current pointer */
	current = inb (cf->resources->port + NE2K_P1_CURR);
	if (nc->next_packet == current)
		return;

	/* set nic to page 0 registers to update boundary register */
	outb (cf->resources->port + NE2K_P1_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_0 | NE2K_CR_STA);

	do {
		/* get pointer to this buffer's header structure */
		packet_ptr = nc->mem_ring + ((nc->next_packet - nc->rec_page_start) << NE2K_PAGE_SHIFT);

		/* the bute count includes a 4 byte header that was added by the nic */
		ne2k_pio_readmem (cf, nc, (uint16_t)packet_ptr, packet_hdr, sizeof (packet_hdr));

		next_packet = packet_hdr[NE2K_RING_NEXT_PACKET];
		len = count = packet_hdr[NE2K_RING_COUNT] +
									256 * packet_hdr[NE2K_RING_COUNT + 1];

		/* recalculate the length for buggy cards */
		if (next_packet >= nc->next_packet)
			nlen = (next_packet - nc->next_packet);
		else
			nlen = ((next_packet - nc->rec_page_start) +
				(nc->rec_page_stop - nc->next_packet));
		--nlen;
		if ((len & NE2K_PAGE_MASK) + sizeof (packet_hdr) > NE2K_PAGE_SIZE)
			--nlen;
		len = (len & NE2K_PAGE_MASK) | (nlen << NE2K_PAGE_SHIFT);
		if (len != count) {
			printf ("%s: length does no match next packet pointer\n", cf->name);
		}

		if (len <= MCLBYTES &&
			next_packet >= nc->rec_page_start &&
			next_packet < nc->rec_page_stop) {
			/* go get packet */
			ne2k_read (cf, nc, packet_ptr + NE2K_RING_HDRSZ, len - NE2K_RING_HDRSZ);
		} else {
			/* ring pointers corrupted! */
			printf ("%s: memory corrupted - invalid packet length %u\n", cf->name, len);
			ne2k_reset (cf, nc);
		}

		/* update the next packet pointer */
		nc->next_packet = next_packet;

		/* update nic boundary pointer */
		boundary = nc->next_packet - 1;
		if (boundary < nc->rec_page_start)
			boundary = nc->rec_page_stop - 1;
		outb (cf->resources->port + NE2K_P0_BNRY, boundary);
	} while (nc->next_packet != current);

	goto loop;
}

/*
 * ne2k_pio_write (MODULECONFIG* cf, NE2K_CONFIG* nc, int len, int buf, char* data)
 *
 * This will write [len] bytes of [dst] to the wire.
 *
 */
int
ne2k_pio_write (MODULECONFIG* cf, NE2K_CONFIG* nc, int len, int dst, uint8_t* data) {
	int maxwait = 120;

	/* select page 0 registers */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_0 | NE2K_CR_STA);

	/* reset remote DMA complete flag */
	outb (cf->resources->port + NE2K_P0_ISR, NE2K_ISR_RDC);

	/* set up DMA byte count */
	outb (cf->resources->port + NE2K_P0_RBCR0, len);
	outb (cf->resources->port + NE2K_P0_RBCR1, len >> 8);

	/* set up destination address in NIC memory */
	outb (cf->resources->port + NE2K_P0_RSAR0, dst);
	outb (cf->resources->port + NE2K_P0_RSAR1, dst >> 8);

	/* set remote DMA write */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD1 | NE2K_CR_PAGE_0 | NE2K_CR_STA);

	/* transfer the data into NIC memory */
	if (!nc->isa16bit) {
		printf ("%s: TODO: 8 bit writes\n", cf->name);
		return 0;
	} else {
		/* NE2000's are trickier */
		int i;
		uint8_t savebyte[2];
		uint16_t d;

		/* 16 bit */
		for (i = 0; i < (len & 0xffff); i += 2) {
			d = (uint16_t)(*(data + i)) | (uint16_t)((*(data + i + 1)) << 8);
			outw (cf->resources->port + 0x10, d);
		}

		if (len & 1) {
			savebyte[0] = (uint8_t)*(data + (len & ~1));
			savebyte[1] = 0;
			outw (cf->resources->port + 0x10, (uint16_t)savebyte);
		}
	}

	/* wait for remote DMA complete */
	while (((inb (cf->resources->port + NE2K_P0_ISR) & NE2K_ISR_RDC) != NE2K_ISR_RDC) &&
				--maxwait);
	if (!maxwait) {
		printf ("%s: remove DMA failed to complete\n", cf->name);
	}

	return len;
}

/*
 * ne2k_xmit (MODULECONFIG* cf, NE2K_CONFIG* nc)
 *
 * This will actually start transmission.
 *
 */
void
ne2k_xmit (MODULECONFIG* cf, NE2K_CONFIG* nc) {
	uint16_t len = nc->txb_len[nc->txb_next_tx];

	/* set page 0 register access */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_0 | NE2K_CR_STA);

	/* set TX buffer start page */
	outb (cf->resources->port + NE2K_P0_TPSR, nc->tx_page_start + nc->txb_next_tx * NE2K_TXBUF_SIZE);

	/* set TX length */
	outb (cf->resources->port + NE2K_P0_TBCR0, len);
	outb (cf->resources->port + NE2K_P0_TBCR1, len >> 8);

	/* set page 0, remote DMA complete, transmit packet, start */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_0 | NE2K_CR_TXP | NE2K_CR_STA);

	/* point to next transmit buffer slot */
	nc->txb_next_tx++;
	if (nc->txb_next_tx == nc->txb_cnt)
		nc->txb_next_tx = 0;
}

/*
 * ne2k_start (MODULECONFIG* cf, NE2K_CONFIG* nc, char* data, int len)
 *
 * This will start output of [len] bytes of [data].
 *
 */
void
ne2k_start (MODULECONFIG* cf, NE2K_CONFIG* nc, char* data, int len) {
	int buffer;

	/* see if there is room to put another packet in the buffer */
	if (nc->txb_inuse == nc->txb_cnt) {
		/* no room. leave */
		printf ("%s: no room in xmit buffer\n", cf->name);
		return;
	}

	/* txb_new points to the next open buffer slot */
	buffer = nc->mem_start + ((nc->txb_new * NE2K_TXBUF_SIZE) << NE2K_PAGE_SHIFT);

	/* write it */
	len = ne2k_pio_write (cf, nc, len, (uint16_t)buffer, data);

	if (len > 60)
		nc->txb_len[nc->txb_new] = len;
	else
		nc->txb_len[nc->txb_new] = 60;

	/* start! */
	ne2k_xmit (cf, nc);

	/* point to next buffer slot */
	if (++nc->txb_new == nc->txb_cnt)
		nc->txb_new = 0;
	nc->txb_inuse++;
}

/*
 * ne2k_irq (MODULECONFIG* cf, NE2K_CONFIG* nc)
 *
 * This will handle incoming IRQ's. 
 *
 */
void
ne2k_irq (MODULECONFIG* cf, NE2K_CONFIG* nc) {
	uint8_t isr;

	/* set the nic to page 0 registers */
	outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_0 | NE2K_CR_STA);

	/* fetch the isr */
	isr = inb (cf->resources->port + NE2K_P0_ISR);
	if (!isr)
		return;

	/* handle them all */
	while (1) {
		/* reset all bits that we are acknowledging */
		outb (cf->resources->port + NE2K_P0_ISR, isr);

		/* handle transmitter interupts first */
		if (isr & (NE2K_ISR_PTX | NE2K_ISR_TXE)) {
			/* read collisions */
			inb (cf->resources->port + NE2K_P0_NCR) /*& 0x0f*/;
			inb (cf->resources->port + NE2K_P0_TSR);
			if (isr & NE2K_ISR_TXE) {
				printf ("%s: transmitter error\n", cf->name);
			} else {
				/* done with the buffer */
				nc->txb_inuse--;
			}
		}

		/* handle receiver interrupts */
		if (isr & (NE2K_ISR_PRX | NE2K_ISR_RXE | NE2K_ISR_OVW)) {
			/* overwrite warning? */
			if (isr & NE2K_ISR_OVW) {
				/* yes. reset the nic */
				printf ("%s: warning - receive ring buffer overrun\n", cf->name);
				ne2k_reset (cf, nc);
			} else {
				/* receiver error */
				if (isr & NE2K_ISR_RXE) {
					printf ("%s: receive error 0x%x\n", cf->name, inb (cf->resources->port + NE2K_P0_RSR));
				}
			}

			/* handle the packet retrieval */
			ne2k_rint (cf, nc);
		}

		/* put the nic back in standard state: page 0, remote DMA complete, start */
		outb (cf->resources->port + NE2K_P0_CR, NE2K_CR_RD2 | NE2K_CR_PAGE_0 | NE2K_CR_STA);

		/* fetch the isr */
		isr = inb (cf->resources->port + NE2K_P0_ISR);
		if (!isr)
			break;
	}
}

/*
 * ne2k_write (struct DEVICE* dev, addr_t offs, uint8_t* data, size_t len)
 *
 * This will transmit [len] bytes from [data] to [dev].
 *
 */
int
ne2k_write (struct DEVICE* dev, addr_t offs, uint8_t* data, size_t len) {
	NE2K_CONFIG* nc = (NE2K_CONFIG*)((struct DEVICE_NETDATA*)dev->data)->data;
	ETHERNET_HEADER* eh = (ETHERNET_HEADER*)data;
	int i;

	/* fill in our MAC source */
	for (i = 0; i < 6; i++)
		eh->source[i] = nc->addr[i];

	/* send it */
	ne2k_start (dev->config, nc, data, len);

	return len;
}

/*
 * main(MODULECONFIG* cf)
 *
 * This will initialize the NE card.
 *
 */
int
main(MODULECONFIG* cf) {
	struct DEVICE dev;
	int i;

	/* allocate the io space */
	if (!register_pio (cf->resources->port, 0x20)) {
		/* this failed. complain */
		printf ("%s: unable to register io ports\n", cf->name);
		exit (0);
	}

	/* do we have a device here? */
	if (modne2k_probe (cf, &edata) < 0) {
		/* no. bail out */
		printf ("%s: NE2000 card not found at io 0x%x irq 0x%x\n", cf->name, cf->resources->port, cf->resources->irq);
		exit (0);
	}

	/* clear the device struct so we can fill out the MAC address */
	memset (&dev, 0, sizeof (struct DEVICE));

	/* the controller seems to exist. attach it */
	dev.type = DEVICE_TYPE_NETWORK;
	dev.name = cf->name;
	dev.config = cf;
	dev.data = &edata;
	dev.write = ne2k_write;
	dev.datasize = sizeof (struct DEVICE_NETDATA);
	edata.device = register_device (&dev);
	if (edata.device == NULL) {
		/* this failed. complain */
		printf ("%s: unable to register device!\n", cf->name);
		exit (0);
	}

	/* fill out our device-specific data */
	for (i = 0; i < 6; i++)
		((struct DEVICE_NETDATA*)edata.device->data)->hw_addr[i] = edata.addr[i];
	((struct DEVICE_NETDATA*)edata.device->data)->data = (void*)&edata;

	/* go! */
	ne2k_init (cf, &edata);

	/* wait for IRQ's */
	if (!register_irq (cf->resources->irq, NULL, NULL)) {
		/* this failed. complain */
		printf ("%s: unable to register irq\n", cf->name);
		exit (0);
	}

	while (1) {
		/* all set, wait until the next IRQ arrives */
		thread_suspend();

		/* handle it */
		ne2k_irq (cf, &edata);
	}
}

/* vim:set ts=2 sw=2: */
