/*
 * ne.h - XeOS NE2000 Module include file
 * (c) 2002 Rink Springer, BSD licensed
 *
 * This module handles all NE2000 device stuff.
 *
 */

#define NE2K_PORT_RESET			0x1f				/* (r) reset port */
#define MCLBYTES					1 << 11

typedef struct {
	uint8_t	isa16bit;										/* 16 bits? */
	int			mem_size;
	int			mem_start;
	int			mem_end;
	int			tx_page_start;
	int			txb_cnt;
	int			txb_inuse;
	int			txb_new;
	int			txb_next_tx;
	int			next_packet;
	uint16_t		txb_len[8];

	int			rec_page_start;
	int			rec_page_stop;
	int			mem_ring;
	uint8_t	addr[6];

	struct	DEVICE*	device;
} NE2K_CONFIG;

/* vim:set ts=2 sw=2: */
