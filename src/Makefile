TARGET	= kernel/kernel.sys
MAKE = gmake

kernel/kernel.sys:
	cd kernel && $(MAKE)

floppy:	kernel/kernel.sys
	cd kernel && $(MAKE) floppy

clean:
	cd kernel && $(MAKE) clean
	cd libc && $(MAKE) clean
	cd modules && $(MAKE) clean
	cd tools && $(MAKE) clean

# vim:set ts=8 sw=8:
