CFLAGS +=	-I../include -nostdlib

include		../$(DEPTH)mk/defs.mk

$(TARGET):	$(OBJS)
		$(AR) rcs $(TARGET) $(OBJS)

clean:
		rm -f $(TARGET) $(OBJS)
