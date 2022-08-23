/*
 * ide.c - XeOS IDE Module
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * Some parts of this code are based on FreeBSD's /sys/i386/isa/wd.c, other
 * parts on http://www.nondot.org/sabre/os/files/Disk/IDE-tech.html and final
 * parts on Yoctix's /sys/modules/bus/isa/wdc/wdc.c.
 *
 * Information on LBA by Hale Landis, from
 * http://www.nondot.org/sabre/os/files/Disk/CHSTranslation.txt
 *
 * This module handles all IDE device stuff.
 *
 */
#include <sys/device.h>
#include <sys/network.h>
#include <sys/types.h>
#include <stdlib.h>
#include "ide.h"

/*
 * send_command (MODULECONFIG* cf, uint8_t cmd, uint8_t drv,
 *                      uint8_t head, uint16_t cyl, uint8_t sector,
 *                      int8_t count)
 *
 * This will send command [cmd] to drive [drv] at [cf], along with all other
 * parameters.
 *
 */
void
send_command (MODULECONFIG* cf, uint8_t cmd, uint8_t drv, uint8_t head, uint16_t cyl, uint8_t sector, uint8_t count) {
	unsigned long timeout;

	/* wait until the drive clears the BUSY bit */
	timeout = 500000;
	while ((inb (cf->resources->port + WD_STATUS) & WDS_BUSY) && (timeout > 0))
		timeout--;

	/* log any drive timeouts */
	if (!timeout)
		printf ("%s: device timeout\n", cf->name);

	/* send the parameters */
	outb (cf->resources->port + WD_SDH, drv * 0x10 + 0xA0 + head);
	outb (cf->resources->port + WD_SECTOR_COUNT, count);
	outb (cf->resources->port + WD_SECTOR, sector);
	outb (cf->resources->port + WD_CYL_LO, (cyl & 0xff));
	outb (cf->resources->port + WD_CYL_HI, (cyl >>   8));

	/* wait for the DRDY bit */
	timeout = 500000;
	while (((inb (cf->resources->port + WD_STATUS) & WDS_READY) == 0) && (timeout > 0))
			timeout--;

	/* log any drive timeouts */
	if (!timeout)
		printf ("%s: device timeout\n", cf->name);

	/* send the command */
	outb (cf->resources->port + WD_COMMAND, cmd);
}

/*
 * wait_reply (MODULECONFIG* cf)
 *
 * This will wait until the IDE device at [cf] sets the DRQ bit.
 *
 */
void
wait_reply (MODULECONFIG* cf) {
	unsigned long timeout;

	/* wait until the drive clears the BUSY bit */
	timeout = 500000;
	while (((inb (cf->resources->port + WD_STATUS) & WDS_BUSY) != 0) && (timeout > 0))
			timeout--;

	/* log any drive timeouts */
	if (!timeout)
		printf ("%s: device timeout\n", cf->name);

	/* wait until the drive sets the DRQ bit */
	timeout = 500000;
	while (((inb (cf->resources->port + WD_STATUS) & WDS_DRQ) == 0) &&
		(timeout > 0))
			timeout--;

	/* log any drive timeouts */
	if (!timeout)
		printf ("%s: device timeout\n", cf->name);
}

/*
 * readbuffer (MODULECONFIG* cf, char* dest, int maxlen)
 *
 * This will keep reading data from the IDE controller in [dest] until the
 * controller said it's done or until we reached [maxlen]. It will return
 * the number of bytes read or 0 on failure.
 *
 */
int
readbuffer (MODULECONFIG* cf, uint16_t* dest, int maxlen) {
	int len = 0;

	/* while there is data, read it */
	for (len = 0; len < 256; len++)
		/* fetch a word */
		dest[len] = inw (cf->resources->port + WD_DATA);
	return len * 2;
}

/*
 * fetch_name (uint16_t* ptr, unsigned int len, char* dest)
 *
 * This will fetch the ascii name from [ptr] in [dest], with a length of [len].
 *
 */
void
fetch_name (uint16_t* ptr, unsigned int len, char* dest) {
	int a = 0;
	int i;

	for (i = 0; i < (len >> 1); i++) {
		dest[a] = ptr[i] >> 8;
		dest[a + 1] = ptr[i] & 0xff;
		a += 2;
	}
	dest[a] = 0; a--;

	while (dest[a] == ' ')
		dest[a--] = 0;
}

/*
 * probe (MODULECONFIG* cf)
 *
 * This will return 0 if there is an IDE controller on [cf], otherwise -1.
 *
 */
int
probe (MODULECONFIG* cf) {
	/* check whether we have registers that work */
	outb (cf->resources->port + WD_SDH, WDM_IBM);	 /* set unit 0 */
	outb (cf->resources->port + WD_CYL_LO, 0xa5);	/* cyl_lo is read/write */
	if (inb (cf->resources->port + WD_CYL_LO) == 0xff) {
		/* humm... maybe the slave device? */
		outb (cf->resources->port + WD_SDH, WDM_IBM | 0x10);		/* set unit 1 */
		outb (cf->resources->port + WD_CYL_LO, 0xa5);
		if (inb (cf->resources->port + WD_CYL_LO) == 0xff)
			/* no such device */
			return -1;
	}

	/* we have a device */
	return 0;
}

/*
 * read_sectors (MODULECONFIG* cf, struct IDE_DISK* dsk, int num,
 *										 int count, char* buf)
 *
 * This will read up to [count] sectors into [buf]. It will start at
 * [num] on drive [drive]. It will return 1 on success or 0 on failure.
 *
 */
int
read_sectors (MODULECONFIG* cf, struct IDE_DISK* dsk, int num, int count, char* buf) {
	/* calculate it to CHS for the old junk which doesn't support LBA */
	int cyl = (num / (dsk->ident.num_heads * dsk->ident.sectors_per_track));
	int tmp = (num % (dsk->ident.num_heads * dsk->ident.sectors_per_track));
	int head = (tmp / dsk->ident.sectors_per_track);
	int sector = (tmp % dsk->ident.sectors_per_track) + 1;

	/* send the command and wait for a reply */
	send_command (cf, WDC_READ, dsk->no, head, cyl, sector, count);
	wait_reply (cf);

	/* fetch the data */
	while (count--) {
		readbuffer (cf, (uint16_t*)buf, 512);
		buf += 512;
	}

	/* all done */	
	return 1;
}

/*
 * ide_partition_read (struct DEVICE* dev, uint64_t offs, void* buf, size_t len) {
 *
 * This will read up to [len] bytes from offset [offs] to [buf]. It will
 * return the number of bytes read.
 *
 */
int
ide_partition_read (struct DEVICE* dev, uint64_t offs, void* buf, size_t len) {
	struct IDE_PARTITION* ip = (struct IDE_PARTITION*)dev->data;

	/* convert bytes to sectors */
	len /= 512;

	/* pass the command to our higher level, but with the correct offsets */
	return (read_sectors (dev->config, ip->dsk, offs + ip->part.abs_no, len, buf) * 512);
}

/*
 * probe_drive (struct DEVICE* core, MODULECONFIG* cf, int drive)
 *
 * This will probe and possibly attach a device to [core] on [cf]. If [drive] is
 * zero, the master drive will be probed, otherwise the slave.
 *
 */
void
probe_drive (struct DEVICE* core, MODULECONFIG* cf, int drive) {
	struct DEVICE dev;
	struct DEVICE* diskdev;
	struct DEVICE* pdev;
	struct IDE_DISK* dsk;
	struct IDE_PARTITION* ip;
	struct PARTITION* part;
	char devname[5] = "hd?";
	uint8_t sector[1024];
	uint8_t model[60];
	uint32_t i;
	size_t l;
	uint16_t* modelptr = (uint16_t*)sector + 0x1b;

	/* probe for the master drive */
	send_command (cf, WDC_IDENTIFY, 0, drive, 0, 0, 0);

	/* wait for the reply */
	wait_reply (cf);

	/* fetch the drive info */
	i = readbuffer (cf, (uint16_t*)sector, sizeof (sector));
	if (i <= 0) {
		printf ("hd%u: device didn't return identification???\n", drive);
		return;
	}

	/* add the device */
	memset (&dev, 0, sizeof (struct DEVICE));
	dev.type = DEVICE_TYPE_BLOCK;
	devname[2] = 0x30 + drive;
	dev.name = devname;
	dev.config = cf;
	dev.parent = core;
	dev.datasize = sizeof (struct IDE_DISK);
	if ((diskdev = (struct DEVICE*)register_device (&dev)) == NULL)
		return;

	/* create a pointer to the device info */
	dsk = (struct IDE_DISK*)diskdev->data;

	/* set the drive number in the struct and copy the identity */
	dsk->no = drive;
	memcpy (&dsk->ident, sector, sizeof (struct IDE_IDENTITY));

	/* fetch the name */
	fetch_name (modelptr, 40, model);

	/* this worked. show info about the drive */
  l = ((dsk->ident.num_cyls * dsk->ident.num_heads * dsk->ident.sectors_per_track) *  dsk->ident.bytes_per_sector) / (1024 * 1024);
	printf ("%s: <%s>, %u MB\n", devname, model, l);

	/* read the MBR */
	read_sectors (cf, dsk, 0, 1, (char*)sector);
	if ((sector[510] != 0x55) || (sector[511] != 0xaa)) {
		/* signature failed. complain */
		printf ("%s: MBR signature mismatch, partition table ignored\n", devname);
		return;
	}

	/* handle all partitions */
	for (i = 0; i < 4; i++) {
		/* store the partition record */
		part = (struct PARTITION*)(sector + 446 + (sizeof (struct PARTITION) * i));

		/* got a partition here? */
		if (part->type) {
			/* yes. register the partition */
			memset (&dev, 0, sizeof (struct DEVICE));
			devname[3] = i + 0x61;
			dev.type = DEVICE_TYPE_BLOCK;
			dev.name = devname;
			dev.config = cf;
			dev.parent = diskdev;
			dev.read = ide_partition_read;
			dev.datasize = sizeof (struct IDE_PARTITION);
			pdev = register_device (&dev);

			/* fill the data out */
			ip = (struct IDE_PARTITION*)pdev->data;
			ip->dev = diskdev;
			ip->dsk = dsk;
			memcpy (&ip->part, part, sizeof (struct PARTITION));

			/* be more verbose */
			printf ("%s: type %x, %u MB @ sector %u\n", devname, part->type, (part->size * dsk->ident.bytes_per_sector) / (1024 * 1024), part->abs_no);
		}
	}

#if 0
	ide_partition_read (pdev, 0, &sector, 1);
	for (i = 0; i < 512; i++)
		if (sector[i] == '\n')
			printf ("[.]");
		else
			printf ("[%c]", sector[i]);
	printf ("\n");
#endif
}

/*
 * probe_drives()
 *
 * This will probe and add drives as needed.
 *
 */
void
probe_drives (struct DEVICE* core, MODULECONFIG* cf) {
	probe_drive (core, cf, 0);
	/*probe_drive (core, cf, 1);*/
}

/*
 * init(MODULECONFIG* cf)
 *
 * This will initialize the IDE device.
 *
 */
int
main(MODULECONFIG* cf) {
	struct DEVICE* core;
	struct DEVICE dev;

	/* allocate the io space */
	if (!register_pio (cf->resources->port, 8)) {
		/* this failed. complain */
		printf ("%s: unable to register io ports\n", cf->name);
		exit (0);
	}

	/* register our IRQ */
	if (!register_irq (cf->resources->irq, NULL, NULL)) {
		/* this failed. complain */
		printf ("%s: unable to register irq\n", cf->name);
		exit (0);
	}

	/* do we have a controller here? */
	if (probe (cf) < 0) {
		/* no. bye */
		printf ("%s: no such device\n", cf->name);
		exit (0);
	}

	/* the controller seems to exist. attach it */
	memset (&dev, 0, sizeof (struct DEVICE));
	dev.type = DEVICE_TYPE_BLOCK;
	dev.name = cf->name;
	dev.config = cf;
	if ((core = register_device (&dev)) == NULL) {
		/* this failed. complain */
		printf ("%s: unable to register device!\n", cf->name);
		exit (0);
	}

	/* search for drives */
	probe_drives (core, cf);

	/* wait until IRQ's arrive */
	while (1) {
		/* all set, wait until the next IRQ arrives */
		thread_suspend();

		/* [we really don't care much about IRQ's yet] */
	}
}

/* vim:set ts=2 sw=2: */
