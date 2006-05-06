# Makefile for frugalwareutils
#
# Copyright (C) 2006 Miklos Vajna <vmiklos@frugalware.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

-include ../Makefile.inc

CFLAGS ?= -g -Wall -O2 -pipe
ifeq ($(shell arch),x86_64)
CFLAGS += -fPIC
endif
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

LDFLAGS += -lparted

# fwutil
CFLAGS += -I../libfwutil
LDFLAGS += -L../libfwutil -lfwutil

compile: libfwraidconfig.so

install:
	$(INSTALL) libfwraidconfig.so.$(VERSION) $(DESTDIR)$(libdir)/libfwraidconfig.so.$(VERSION)
	ln -sf libfwraidconfig.so.$(VERSION) $(DESTDIR)$(libdir)/libfwraidconfig.so.$(MAJORVER)
	ln -sf libfwraidconfig.so.$(VERSION) $(DESTDIR)$(libdir)/libfwraidconfig.so

clean:
	rm -f *.so*

%.so: %.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(basename $@).so.$(MAJORVER) -o \
		$(basename $@).so.$(VERSION) $(basename $@).o $(LDFLAGS)
	ln -sf $(basename $@).so.$(VERSION) $(basename $@).so.$(MAJORVER)
	ln -sf $(basename $@).so.$(VERSION) $(basename $@).so
