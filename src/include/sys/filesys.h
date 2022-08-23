/*
 * filesys.h - XeOS Filesystem Manager
 * (c) 2002 Rink Springer, BSD licensed
 *
 * This code will handle filesystems.
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/thread.h>
#include <md/semaphore.h>

#ifndef __FILESYS_H__
#define __FILESYS_H__

#define HANDLE_FSDEP_LEN	64

/*
 * HANDLE describes a file or directory handle.
 */
struct HANDLE {
	struct MOUNTEDFS* mfs;
	uint64_t	start;

	uint8_t fsdep[HANDLE_FSDEP_LEN];
};

struct FILESYSTEM {
	char*	name;
	struct THREAD* thread;
	struct FILESYSTEM* next;

	/* fetch the desired of the workbuffer */
	size_t (*get_workbuf_size)();

	/* mount a filesystem */ 
  uint32_t (*mount)(struct DEVICE* dev, void* workbuf, uint32_t flags);

	/* unmount a filesystem */ 
  size_t (*umount)(struct DEVICE* dev, void* workbuf);

	/* open a file or directory */
  size_t (*open)(struct DEVICE* dev, void* workbuf, char* path, struct HANDLE* h, uint32_t flags);
};

struct MOUNTEDFS {
	char* mountpoint;
	struct DEVICE* device;
	SEMAPHORE sem;
	struct FILESYSTEM* fs;
	uint32_t flags;
	void* workbuf;
	struct MOUNTEDFS* next;
};

#ifdef __KERNEL
int	fs_register (struct THREAD* t, struct FILESYSTEM* fs);
int	fs_mount (char* dev, char* mp, char* fstype, uint32_t flags);
void fs_dump();
#endif /* __KERNEL */

#endif /* __FILESYS_H__ */

/* vim:set ts=2 sw=2: */
