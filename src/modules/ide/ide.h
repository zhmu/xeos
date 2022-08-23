/*
 * ide.h - XeOS IDE Module include file
 * (c) 2002 Rink Springer, BSD licensed
 *
 * This module handles all IDE device stuff.
 *
 */

/* ports */
#define WD_DATA		0x00			/* (R/W) data port */
#define WD_ERROR	0x01			/* (R) error register */
#define WD_PRECOMP	WD_ERROR		/* (W) write precompensation */
#define WD_FEATURES	WD_ERROR		/* (W) features register */
#define WD_SECTOR_COUNT	0x02			/* (R/W) sector count */
#define WD_SECTOR	0x03			/* (R/W) first sector number */
#define WD_CYL_LO	0x04			/* (R/W) cyl address, lo byte */
#define WD_CYL_HI	0x05			/* (R/W) cyl address, hi byte */
#define WD_SDH		0x06			/* (R/W) sec/drive/header */
#define WD_COMMAND	0x07			/* (W) command register */
#define WD_STATUS	WD_COMMAND		/* (R) status register */

/* status bits */
#define WDS_BUSY	0x80			/* controller is busy */
#define WDS_READY	0x40			/* selected drive is ready */
#define WDS_WRTFLT	0x20			/* write fault */
#define WDS_SEEKCPL	0x10			/* seek completed */
#define WDS_DRQ		0x08			/* data request bit */
#define WDS_ECC		0x04			/* ecc correction made */
#define WDS_INDEX	0x02			/* index pulse from drive */
#define WDS_ERROR	0x01			/* error bit */

/* commands */
#define WDC_RESTORE	0x10			/* restore disk */
#define WDC_READ	0x20			/* read disk */
#define WDC_WRITE	0x30			/* write disk */
#define WDC_FORMAT	0x50			/* format disk */
#define WDC_DIAGNOSE	0x90			/* controller diagnosis */
#define WDC_INIT	0x91			/* initialize drive */
#define WDC_READ_MULTI	0xC4			/* read multi sectors */
#define WDC_WRITE_MULTI	0xC5			/* write multi sectors */
#define WDC_SET_MULTI	0xC6			/* set multi count */
#define WDC_READ_DMA	0xC8			/* read using DMA */
#define WDC_WRITE_DMA	0xCA			/* write using DMA */

#define WDC_EXTCMD	0xE0			/* extended command */
#define WDC_IDENTIFY	0xEC			/* read parameters */
#define WDC_FEATURES	0xEF			/* features control */

/* modifiers */
#define WDM_IBM		0xA0			/* forced to 512 byte sectors */
#define WDM_LBA		0x40			/* use LBA */

struct IDE_IDENTITY {
    uint16_t		config;			/* 00-configuration field */
    uint16_t		num_cyls;		/* 01-number of cylinders */
    uint16_t		res0;			/* 02-reserved */
    uint16_t		num_heads;		/* 03-number of heads */
    uint16_t		bytes_per_track;	/* 04-bytes per track */
    uint16_t		bytes_per_sector;	/* 05-bytes per sector */
    uint16_t		sectors_per_track;	/* 06-sectors per track */
    uint16_t		gap;			/* 07-gap */
    uint16_t		phase;			/* 08-phase lock field */
    uint16_t		num_vendor_words;	/* 09-number of vendor words */
    uint8_t		serial_number[20];	/* 0A-serial number */
    uint16_t		controller_type;	/* 14-controller type */
    uint16_t		bufsize;		/* 15-buffer size */
    uint16_t		ecc_bytes;		/* 16-number of ECC bytes */
    uint8_t		firmware[8];		/* firmware version */
    uint8_t		model[40];		/* model number */
} __attribute__((packed));

struct PARTITION {
	uint8_t		flags;							/* flags */
	uint8_t		start_head;					/* starting head */
	uint8_t		start_sector;				/* starting sector */
	uint8_t		start_cyl;					/* starting cylinder */
	uint8_t		type;								/* partition type */
	uint8_t		end_head;						/* end head */
	uint8_t		end_sector;					/* end sector */
	uint8_t		end_cyl;						/* end cylinder */
	uint32_t	abs_no;							/* absolute sector number */
	uint32_t	size;								/* partition size */
} __attribute__((packed));

struct IDE_DISK {
	int no;
	struct IDE_IDENTITY ident;
};

struct IDE_PARTITION {
	struct DEVICE* dev;
	struct IDE_DISK* dsk;
	struct PARTITION part;
};
