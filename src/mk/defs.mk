# standard executables
LD		= ld
CP		= cp
STRIP		= strip
RM		= rm
MV		= mv
AS		= as
GZ		= gzip
MAKE		= gmake

# stuff we build ourselves
SYS2C		= ../tools/sys2c

# c files
.c.o:	      
		$(CC) $(CFLAGS) -c -o $@ $<

# assembly files
.s.o:
		$(AS) $(SFLAGS) -o $@ $<

