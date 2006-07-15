compile: $(CONFNAME).so
	+$(DO_RECURSIVE)

install:
	$(INSTALL) $(CONFNAME).so $(DESTDIR)$(fwlibdir)/$(CONFNAME).so
	ln -sf setup $(DESTDIR)$(sbindir)/$(CONFNAME)
	+$(DO_RECURSIVE)

clean:
	rm -f $(CONFNAME).so *.o
	+$(DO_RECURSIVE)

%.so: %.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(basename $@).so -o \
		$(basename $@).so $(basename $@).o $(LDFLAGS)

# vim: ft=make
