/*
 * filesys.c - XeOS Filesystem Manager
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle filesystems.
 *
 */
#include <sys/filesys.h>
#include <sys/thread.h>
#include <sys/kmalloc.h>
#include <sys/types.h>
#include <lib/lib.h>
#include <md/interrupts.h>
#include <md/vm.h>
#include <assert.h>

struct FILESYSTEM* fs_rootchain = NULL;
struct MOUNTEDFS* fs_mountchain = NULL;

/*
 * fs_register (struct THREAD* t, struct FILESYSTEM* regfs)
 *
 * This will register filesystem [regfs]. It will return 0 on failure or
 * non-zero on success.
 */
int
fs_register (struct THREAD* t, struct FILESYSTEM* regfs) {
	struct FILESYSTEM* scanfs = fs_rootchain;
	struct FILESYSTEM* fs;

	/* allocate memory and set the filesystem up */
	fs = (struct FILESYSTEM*)kmalloc (t, sizeof (struct FILESYSTEM), 0);
	kmemcpy (fs, regfs, sizeof (struct FILESYSTEM));

	/* fix the thread and next pointers */
	fs->name = kstrdup (regfs->name);
	fs->thread = t;
	fs->next = NULL;

	/* got a root filesystem? */
	if (fs_rootchain == NULL) {
		/* no. now, we do */
		fs_rootchain = fs;
	} else {
		/* yes. append this filesystem to the chain */
		while (scanfs->next)
			scanfs = scanfs->next;
		scanfs->next = fs;
	}

	/* all done */
	return 1;
}

/*
 * fs_find (char* name)
 *
 * This will scan for filesystem [name]. It will return a pointer to the
 * filesystem on success or NULL on failure.
 *
 */
struct FILESYSTEM*
fs_find (char* name) {
	struct FILESYSTEM* fs = fs_rootchain;

	/* scan them all */
	while (fs) {
		/* match? */
		if (!kstrcmp (fs->name, name))
			/* yes. got it */
			return fs;

		/* next */
		fs = fs->next;
	}

	/* too bad, so sad */
	return NULL;
}

/*
 * fs_mount (char* devname, char* mp, char* fstype, uint32_t flags);
 *
 * This will mount device [dev] as [fstype] on [mp] using flags [flags].
 *
 */
int
fs_mount (char* devname, char* mp, char* fstype, uint32_t flags) {
	struct DEVICE* dev = device_find (devname);
	struct FILESYSTEM* fs = fs_find (fstype);
	void* workbuf = NULL;
	size_t workbufsize;
	addr_t addr = arch_vm_getmap();
	uint32_t i;
	struct MOUNTEDFS* mfs;
	struct MOUNTEDFS* tmpmfs;

	/* got a device and filesystem? */
	if ((dev == NULL) || (fs == NULL))
		/* no. fail */
		return 0;

	/* fetch the size to allocate */
	arch_vm_setthreadmap (fs->thread);
	workbufsize = fs->get_workbuf_size();
	arch_vm_setmap (addr);
	if (workbufsize)
		/* allocate it */
		workbuf = kmalloc (NULL, workbufsize, 0);

	/* switch to the device's memory table */
	arch_vm_setthreadmap (fs->thread);

	/* do the mount */
	i = fs->mount (dev, workbuf, flags);

	/* switch back to the original thread's memory table */
	arch_vm_setmap (addr);

	/* failure? */
	if (!i) {
		/* yes. free the workbuf and return */
		if (workbuf) kfree (workbuf);
		return 0;
	}

	/* set the structure up */
	mfs = (struct MOUNTEDFS*)kmalloc (NULL, sizeof (struct MOUNTEDFS), 0);
	mfs->mountpoint = kstrdup (mp);
	arch_sem_create (&mfs->sem);
	mfs->fs = fs;
	mfs->device = dev;
	mfs->flags = flags;
	mfs->workbuf = workbuf;
	mfs->next = NULL;
	
	/* do we have allocated filesystems? */
	if (fs_mountchain == NULL)
		/* no. now, we do */
		fs_mountchain = mfs;
	else {
		/* yes. append it */
		tmpmfs = fs_mountchain;
		while (tmpmfs->next)
			tmpmfs = tmpmfs->next;
		tmpmfs->next = mfs;
	}

	/* all done */
	return 1;
}

/*
 * fs_get_filesystem (char* path)
 *
 * This will fetch the mounted filesystem residing with path [path]. It will
 * return NULL on failure, otherwise the entry.
 *
 */
struct MOUNTEDFS*
fs_get_filesystem (char* path) {
	char* ptr = path;
	struct MOUNTEDFS* mfs;

	/* if the first char of the path isn't a slash, bail out */
	if (*path != '/')
		return NULL;

	/* go to the end of the path */
	while (*ptr) ptr++;

	/* now, scan from right to left */
	while (ptr != path) {
		/* scan for a / */
		while (*ptr != '/') ptr--;

		/* scan the mounted table for this path */
		mfs = fs_mountchain;
		while (mfs) {
			/* got a match? */
#if 0
			kprintf ("comparing: [%s] vs [%s], %x len\n", 
			path, mfs->mountpoint, (ptr - path));
#endif
			if (!kstrncmp (path, mfs->mountpoint, (ptr - path)))
					/* yes! return it */
					return mfs;

			/* next */
			mfs = mfs->next;
		}

		/* next */
		ptr--;
		if (!*ptr) break;
	}

	/* ??? */
	return NULL;
}

/*
 * fs_open (struct HANDLE* h, char* path, uint32_t flags)
 *
 * This will open file or directory [path] in handle [h] with flags [flags]. It
 * will return zero on failure or non-zero on success.
 *
 */
int
fs_open (struct HANDLE* h, char* path, uint32_t flags) {
	struct MOUNTEDFS* mfs = fs_get_filesystem (path);
	addr_t addr = arch_vm_getmap();
	int i;

	/* this shouldn't happen... */
	ASSERT (mfs != NULL);

	/* tell the filesystem to open the directory */
	arch_vm_setthreadmap (mfs->fs->thread);
	i = mfs->fs->open (mfs->device, mfs->workbuf, path, h, flags);
	arch_vm_setmap (addr);

	/* got a valid handle and success? */
	if ((h != NULL) && (i)) {
		/* yes. ensure the mounted fs field points to us */
		h->mfs = mfs;
	}
	return i;
}

/*
 * fs_dump()
 *
 * This will do a debugging dump of the filesystems.
 *
 */
void
fs_dump() {
	struct FILESYSTEM* fs = fs_rootchain;
	struct MOUNTEDFS* mfs = fs_mountchain;

	kprintf ("Loaded filesystem dump:\n");
	while (fs) {
		kprintf ("%s: thread %x\n", fs->name, fs->thread);

		/* next */
		fs = fs->next;
	}

	kprintf ("Mounteded filesystem dump:\n");
	while (mfs) {
		kprintf ("device %x mounted on %s, type %s\n", mfs->device, mfs->mountpoint, mfs->fs->name);

		/* next */
		mfs = mfs->next;
	}


	kprintf ("debugging opendir -> %x\n", fs_open (NULL, "/BIN/NWCLIENT/LSL.COM", 0));
}

/* vim:set ts=2 sw=2: */
