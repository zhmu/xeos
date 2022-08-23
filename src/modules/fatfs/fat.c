/*
 * fat.c - XeOS FAT Filesystem Driver
 *
 * (c) 2003 Rink Springer, BSD
 *
 * Cluster information from
 * http://www.nondot.org/sabre/os/files/FileSystems/fatFilesystem.txt,
 * (c) Inbar Raz
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/filesys.h>
#include <sys/network.h>
#include "fat.h"

#define xFATFS_OPEN_DEBUG
#define xFATFS_CLUSTER_DEBUG

/*
 * fatfs_get_workbufsize()
 *
 * This will return the number of bytes we need for our work buffer.
 *
 */
size_t
fatfs_get_workbufsize() {
	return sizeof (struct FAT_WORKBUFFER) + 4096;
}

/*
 * fatfs_mount (struct DEVICE* dev, void* workbuf, uint32_t flags)
 *
 * This will try to mount [dev]. It will return 0 on failure or non-zero on
 * success.
 *
 */
uint32_t
fatfs_mount (struct DEVICE* dev, void* workbuf, uint32_t flags) {
	struct FAT_WORKBUFFER* wbuf = (struct FAT_WORKBUFFER*)workbuf;
	struct FAT_BOOTSECTOR* boot = (struct FAT_BOOTSECTOR*)&wbuf->scratch;
	uint32_t i, j;

	/* read the very first sector */
	if (!device_read (dev, 0, &wbuf->scratch, 512))
		return 0;

	/* convert the bootsector */
	wbuf->sector_size       = boot->sec_size[0]   | (boot->sec_size[1]   << 8);
	wbuf->dir_entries       = boot->dir_ents[0]   | (boot->dir_ents[1]   << 8);
	wbuf->total_sectors     = boot->tot_sect[0]   | (boot->tot_sect[1]   << 8);
	wbuf->sectors_per_track = boot->spt[0]        | (boot->spt[1]        << 8);
	wbuf->num_heads         = boot->num_heads[0]  | (boot->num_heads[1]  << 8);
	wbuf->num_hidden        = boot->num_hidden[0] | (boot->num_hidden[1] << 8);
	wbuf->num_reserved      = boot->num_res[0]    | (boot->num_res[1]    << 8);
	wbuf->fat_size          = boot->fat_size[0]   | (boot->fat_size[1]   << 8);
	wbuf->cluster_size      = boot->cl_size;
	wbuf->num_fats          = boot->numfats;

	/* calculate our own pointers */
	wbuf->rootdir_sector	  = (wbuf->fat_size * wbuf->num_fats) + wbuf->num_reserved;

	/* sector size of 512 and a jump? */
	if ((wbuf->sector_size != 0x200) || ((boot->jump[0] >> 4) != 0xe)) {
		/* no. complain */
		printf ("fatfs_mount(): doesn't look like FAT, not mounting\n");
		return 0;
	}

	printf ("fatfs_mount(): mount ok, clustersize %x sectors\n", wbuf->cluster_size);

#if 0

	/* root dir dump! */
	for (i = 0; i < (wbuf->dir_entries / 32); i++) {
		if (!device_read (dev, wbuf->rootdir_sector + i, &sector, 512))
			return 0;
		fe = (struct FAT_ENTRY*)sector;

		for (j = 0; j < (wbuf->sector_size / 32); j++) {
			if (fe->name[0]) {
				printf ("file: [%c%c%c%c%c%c%c%c.%c%c%c]\n",
						fe->name[0], fe->name[1], fe->name[2], fe->name[3], fe->name[4], fe->name[5], fe->name[6],
						fe->name[7], fe->ext[0], fe->ext[1], fe->ext[2]);
			} 
			fe++;
		}
	}
#endif

	/* victory */
	return 1;
}

/*
 * conv_cluster_to_sector (struct FAT_WORKBUFFER* wbuf, uint32_t clusterno)
 *
 * This will return the sector number of [clusterno].
 *
 */
uint32_t
conv_cluster_to_sector (struct FAT_WORKBUFFER* wbuf, uint32_t clusterno) {
	uint32_t cno;

	cno =  ((clusterno - 2) * wbuf->cluster_size) + wbuf->rootdir_sector;
	cno +=  ((wbuf->dir_entries * 32) / wbuf->sector_size);
#if 0
	printf ("clusterno %x -> %x\n", clusterno, cno);
#endif
	return cno;
}

/*
 * get_next_cluster (struct DEVICE* dev, struct FAT_WORKBUFFER* wbuf,
 *                   uint32_t clusterno)
 *
 * This will return the next cluster of [clusterno].
 *
 */
uint32_t
get_next_cluster (struct DEVICE* dev, struct FAT_WORKBUFFER* wbuf, uint32_t clusterno) {
	char* sector = (char*)&wbuf->scratch;

	if (!device_read (dev, wbuf->num_reserved + (clusterno / 256), sector, wbuf->sector_size))
		return -1;

	/* return the cluster */
#ifdef FATFS_CLUSTER_DEBUG
	printf ("getnextcluster(): cluster %x -> %x\n", clusterno,
	 (sector[(clusterno % 256) * 2] | (sector[((clusterno % 256) * 2) + 1] << 8)));
#endif
	return (sector[(clusterno % 256) * 2] | (sector[((clusterno % 256) * 2) + 1] << 8));
}

/*
 * scan_dir_sector (struct FAT_WORKBUFFER* wbuf, char* sector, char* fname,
 *                  size_t len)
 *
 * This will return a pointer to the FAT entry of [fname] in [sector], or NULL
 * if it couldn't be found.
 *
 */
struct FAT_ENTRY*
scan_dir_sector (struct FAT_WORKBUFFER* wbuf, char* sector, char* fname, size_t len) {
	struct FAT_ENTRY* fe = (struct FAT_ENTRY*)sector;
	uint32_t i, j;
	char* dot = NULL;
	char* ptr; 

	/* isolate the '.' */
	ptr = fname;
	for (i = 0; i < len; i++)
		if (ptr[i] == '.') {
			printf ("found '.' at pos %x, len=%x\n", i, len);
			dot = (ptr + i);
			len = i;
			break;
		}

	/* scan the entire sector */
	for (j = 0; j < (wbuf->cluster_size * (wbuf->sector_size / 32)); j++) {
		/* replace all spaces by NULL's */
		for (i = 0; i < 7; i++)
			if (fe->name[i] == 0x20)
				fe->name[i] = 0;

#ifdef FATFS_OPEN_DEBUG
				printf ("file: [%c%c%c%c%c%c%c%c.%c%c%c] -> ",
						fe->name[0], fe->name[1], fe->name[2], fe->name[3], fe->name[4], fe->name[5], fe->name[6],
						fe->name[7], fe->ext[0], fe->ext[1], fe->ext[2]);

		/* got a match? */
		printf ("strcmp(): [%s], [%s.%s], %x\n", fname, fe->name, fe->ext, len);
#endif /* FATFS_OPEN_DEBUG */
		if (!strncmp (fname, fe->name, len)) {
			/* yes! need to check for an extension? */
			if (dot != NULL) {
				/* yes. match? */
#ifdef FATFS_OPEN_DEBUG
				printf ("dot.strcmp(): [%s], [%s]\n", dot + 1, fe->ext);
#endif /* FATFS_OPEN_DEBUG */
				if (!strncmp (dot + 1, fe->ext, 3)) {
					/* yes! */
					return fe;
				}
			} else {
				return fe;
			}
		}

		/* next */
		fe++;
	}

	/* not found */
	return NULL;
}

/*
 * fatfs_open (struct DEVICE* dev, void* workbuf, char* path, struct HANDLE* h,
 *             uint32_t flags)
 *
 * This will open file or directory [path].
 *
 */
int
fatfs_open (struct DEVICE* dev, void* workbuf, char* path, struct HANDLE* h, uint32_t flags) {
	struct FAT_WORKBUFFER* wbuf = (struct FAT_WORKBUFFER*)workbuf;
	char* sector = (char*)&wbuf->scratch;
	uint32_t cur_dirsector = wbuf->rootdir_sector;
	uint32_t root_sectors = (wbuf->dir_entries / 32);
	struct FAT_ENTRY* fe;
	char* curptr = path + 1;
	char* ptr;
	uint32_t i, j;

	/* scan through the entire directory tree */
	while (1) {
		/* fetch the sector */
		if (root_sectors)
			i = device_read (dev, cur_dirsector, sector, wbuf->sector_size);
		else {
			for (j = 0; j < wbuf->cluster_size; j++) {
				i = device_read (dev, conv_cluster_to_sector (wbuf, cur_dirsector) + j, sector + (j * wbuf->sector_size), wbuf->sector_size);
			}
		}

		/* on failure, bail out */
		if (!i)
			return 0;

		/* scan for the first / */
		ptr = strchr (curptr, '/');
		if (ptr == NULL) {
			/* humm, not found. scan till the end, then */
			ptr = strchr (curptr, 0);
		}

		/* scan for the file */
		fe = scan_dir_sector (wbuf, sector, curptr, ptr - curptr);
		if (fe != NULL) {
			/* yay, we got a match! do we need more to match? */
			if (!*ptr)
				/* no. get out of this while loop */
				break;
			
			/* continue to this sector */
			cur_dirsector = fe->lo_clus[0] | (fe->lo_clus[1] << 8);
#ifdef FATFS_CLUSTER_DEBUG
			printf ("new sector: %x\n", cur_dirsector);
#endif /* FATFS_CLUSTER_DEBUG */
		  root_sectors = 0; curptr = ptr + 1;
			continue;
		}

		/* next */
		if (root_sectors)
			root_sectors--;
		else {
			/* fetch the next cluster */
			cur_dirsector = get_next_cluster (dev, wbuf, cur_dirsector);

			/* end of the line? */
			if (cur_dirsector & 0xFFF0) {
				/* yes. bail out */
				return 0;
			}
		}
	}

	/* we are victorious! need to set a handle up? */
	if (h != NULL)
		/* yes. do it */
		memcpy (h->fsdep, fe, sizeof (struct FAT_ENTRY));

	return 1;
}

int
main() {
	struct FILESYSTEM fs;

	/* set the filesystem structure up */
	memset (&fs, 0, sizeof (struct FILESYSTEM));
	fs.name = "fatfs";
	fs.get_workbuf_size = &fatfs_get_workbufsize;
	fs.mount = &fatfs_mount;
	fs.open = &fatfs_open;

	/* register the filesystem */
	if (!register_filesystem (&fs)) {
		/* this failed. complain */
		printf ("fatfs: unable to register filesystem, exiting\n");
		exit (0);
	}

	while (1)
		/* zzzz */
		thread_suspend();
}

/* vim:set ts=2 sw=2: */
