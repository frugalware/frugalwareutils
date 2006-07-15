compile: libfw$(LIBNAME).so
	+$(DO_RECURSIVE)

install:
	$(INSTALL) libfw$(LIBNAME).so.$(VERSION) $(DESTDIR)$(libdir)/libfw$(LIBNAME).so.$(VERSION)
	ln -sf libfw$(LIBNAME).so.$(VERSION) $(DESTDIR)$(libdir)/libfw$(LIBNAME).so.$(MAJORVER)
	ln -sf libfw$(LIBNAME).so.$(VERSION) $(DESTDIR)$(libdir)/libfw$(LIBNAME).so
	+$(DO_RECURSIVE)

clean:
	rm -f *.so*
	+$(DO_RECURSIVE)

%.so: %.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(basename $@).so.$(MAJORVER) -o \
		$(basename $@).so.$(VERSION) $(basename $@).o $(LDFLAGS)
	ln -sf $(basename $@).so.$(VERSION) $(basename $@).so.$(MAJORVER)
	ln -sf $(basename $@).so.$(VERSION) $(basename $@).so

# vim: ft=make
