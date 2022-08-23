CFLAGS	= -I../../include
LIBS    = ../../libc/libc.a

include		../$(DEPTH)mk/defs.mk

%.sys:		$(OBJS)
		#$(LD) -static -o $@ -e main $(OBJS) -nostdlib -lc -L../../libc
		$(LD) -nostdlib -o $@ -e main $(OBJS) $(LIBS)
		$(CP) $@ $@.debug
		$(STRIP) --remove-section=.comment --remove-section=.note $@

clean:
		$(RM) -f $@ $@.debug $(OBJS)

# vim:set ts=8 sw=8:
