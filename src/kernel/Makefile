TARGET  = kernel.sys
OBJS	= main/main.o \
	arch/i386/stub.o arch/i386/init.o arch/i386/memory.o \
	arch/i386/timer_asm.o arch/i386/halt.o arch/i386/syscall_asm.o \
	arch/i386/vm.o arch/i386/startup.o arch/i386/task.o \
	arch/i386/semaphore.o arch/i386/int_asm.o arch/i386/exceptions.o \
	arch/i386/interrupts.o arch/i386/console.o \
	kern/syscall.o kern/sys_io.o kern/sys_mbox.o kern/sys_module.o \
	kern/sys_thread.o kern/sys_debug.o kern/sys_network.o \
	kern/sys_filesys.o \
	sys/irq.o sys/thread.o sys/device.o sys/kmalloc.o sys/pio.o \
	sys/tty.o sys/elf.o sys/mailbox.o sys/network.o sys/filesys.o \
	lib/kprintf.o lib/panic.o lib/string.o \
	lib/i386/htonl.o lib/i386/htons.o lib/i386/kmemcmp.o \
	lib/i386/memcpy.o lib/i386/memset.o lib/i386/ntohl.o \
	lib/i386/ntohs.o lib/i386/strcat.o lib/i386/strchr.o \
	lib/i386/strcmp.o lib/i386/strcpy.o lib/i386/strlen.o \
	lib/i386/strncmp.o
CFLAGS	= -nostdinc -I../include -Wall -Werror
CFLAGS	+= -D__KERNEL -DARCH=i386
#CFLAGS	+= -DHAVE_DISASM
SFLAGS +=	-Ilib/i386

include		../mk/defs.mk

.S.o:
		$(CC) $(SFLAGS) -c -o $@ $<

.ORDER:		version.h kernel.sys

kernel.sys:	version.h $(OBJS) $(LIBS)
		$(LD) -Ttext 0x80100000 -e __start -o kernel.sys $(OBJS) $(LIBS)
		$(CP) kernel.sys kernel.debug.sys
		$(STRIP) --remove-section=.comment --remove-section=.note kernel.sys
		$(RM) -f version.h

clean:
		$(RM) -f kernel.sys kernel.debug.sys version.h $(OBJS)

GENERATION:
		echo -1 > GENERATION

version.h:	Makefile GENERATION
		echo `cat GENERATION`+1|bc > GENERATION.tmp
		mv -f GENERATION.tmp GENERATION
		@echo "#define VERSION \"XeOS #`cat GENERATION`: `date`\n  `whoami`@`hostname`:`pwd`\"" > version.h

floppy:		kernel.sys ../tools/base-grub.flp
		$(CP) ../tools/base-grub.flp /tmp/xeos.flp
		sudo mount /tmp/xeos.flp /mnt -t msdos -o loop,uid=rink,sync
		$(CP) kernel.sys /mnt/boot/xeos
		sudo umount /mnt

main/main.c:		modules/modules.c

modules/modules.c:	../modules/Makefile
			cd ../modules && $(MAKE)

../tools/base-grub.flp:	../tools/Makefile
			cd ../tools && $(MAKE) base-grub.flp

copy:			kernel.sys
			mcopy -o kernel.sys a:/boot/xeos

# vim:set ts=8 sw=8:
