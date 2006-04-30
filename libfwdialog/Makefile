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
LDFLAGS=-ldialog -lcurses -lm
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

# fwutil
CFLAGS += -I../libfwutil
LDFLAGS += -L../libfwutil -lfwutil

SUBDIRS = po

compile: libfwdialog.so

install:
	$(INSTALL) libfwdialog.so.$(VERSION) $(DESTDIR)$(libdir)/libfwdialog.so.$(VERSION)
	ln -sf libfwdialog.so.$(VERSION) $(DESTDIR)$(libdir)/libfwdialog.so.$(MAJORVER)
	ln -sf libfwdialog.so.$(VERSION) $(DESTDIR)$(libdir)/libfwdialog.so
	$(DO_RECURSIVE)

clean:
	rm -f *.so*
	$(DO_RECURSIVE)

%.so: %.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(basename $@).so.$(MAJORVER) -o \
		$(basename $@).so.$(VERSION) $(basename $@).o $(LDFLAGS)
	ln -sf $(basename $@).so.$(VERSION) $(basename $@).so.$(MAJORVER)
	ln -sf $(basename $@).so.$(VERSION) $(basename $@).so
