/*
 * fat.h - XeOS FAT Filesystem Driver
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 */
#include <sys/types.h>

#ifndef __FAT_H__
#define __FAT_H__

/*
 * based on msdos.h from mtools
 */
struct FAT_BOOTSECTOR {
	uint8_t	jump[3];	/* 00: jump to boot code */
	uint8_t	banner[8];	/* 03: oem name and version */
	uint8_t sec_size[2];	/* 11: sector size */
	uint8_t cl_size;	/* 13: cluster size in sectors */
	uint8_t num_res[2];	/* 14: number of reserved sectors */
	uint8_t numfats;	/* 16: number of fats */
	uint8_t dir_ents[2];	/* 17: number of dir slots */
	uint8_t tot_sect[2];	/* 19: total sectors on disk */
	uint8_t desc;		/* 21: media descriptor */
	uint8_t fat_size[2];	/* 22: sectors in FAT */
	uint8_t spt[2];		/* 24: sectors per track */
	uint8_t num_heads[2];	/* 26: number of heads */
	uint8_t num_hidden[2];	/* 28: number of hidden sectors */
	uint8_t bigsect[4];	/* 32: number of 'big' sectors */
} __attribute__((packed));

/*
 * based on direntry.h from FreeBSD's /sys/msdosfs/direntry.h
 */
struct FAT_ENTRY {
	uint8_t name[8];	/* filename */
	uint8_t ext[3];		/* extension */
	uint8_t	attr;		/* attributes */
	uint8_t	lcase;		/* weird NT flag */
	uint8_t	hund_time;	/* hundred's of seconds creation time */
	uint8_t ctime[2];	/* create time */
	uint8_t cdate[2];	/* create date */
	uint8_t adate[2];	/* access date */
	uint8_t hi_clus[2];	/* high cluster number */
	uint8_t mtime[2];	/* modification time */
	uint8_t mdate[2];	/* modification date */
	uint8_t lo_clus[2];	/* high cluster number */
	uint8_t	size[4];	/* size */
} __attribute__((packed));

struct FAT_WORKBUFFER {
	uint16_t sector_size;
	uint8_t	 cluster_size;
	uint8_t  num_fats;
	uint16_t dir_entries;
	uint16_t total_sectors;
	uint16_t fat_size;
	uint16_t num_heads;
	uint16_t sectors_per_track;
	uint16_t num_hidden;
	uint16_t num_reserved;

	uint32_t rootdir_sector;

	uint8_t	 scratch;
};

#endif /* __FAT_H__ */
