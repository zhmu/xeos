TARGET =	libc.a
OBJS =		arch/i386/syscall.o arch/i386/pio.o \
                kern/syscalls.o \
                lib/output.o lib/string.o lib/input.o lib/conv.o lib/main.o \
		lib/i386/htonl.o lib/i386/htons.o lib/i386/kmemcmp.o \
		lib/i386/memcpy.o lib/i386/memset.o lib/i386/ntohl.o \
		lib/i386/ntohs.o lib/i386/strcat.o lib/i386/strchr.o \
		lib/i386/strcmp.o lib/i386/strcpy.o lib/i386/strlen.o \
		lib/i386/strncmp.o
CFLAGS +=	-I../include -nostdlib -D__LIBC
SFLAGS +=	-Ilib/i386

include ../mk/lib.mk
include ../mk/defs.mk

#libc.a:		$(OBJS)
#		ld -o libc.a -nostdlib -shared $(OBJS)


kern/syscalls.o:	kern/SYSCALLS
		cd kern && $(MAKE)

clean:
		rm -f libc.a $(OBJS)

.S.o:
		$(CC) $(SFLAGS) -c -o $@ $<

