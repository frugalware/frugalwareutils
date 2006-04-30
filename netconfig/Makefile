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

CFLAGS ?= -g -Wall -O2 -pipe
LDFLAGS=-ldialog -lcurses -lm
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

# fwutil
CFLAGS += -I../libfwutil
LDFLAGS += -L../libfwutil -lfwutil

# fwdialog
CFLAGS += -I../libfwdialog
LDFLAGS += -L../libfwdialog -lfwdialog

# fwnetconfig
CFLAGS += -I../libfwnetconfig
LDFLAGS += -L../libfwnetconfig -lfwnetconfig

# setup
CFLAGS += -I../setup

-include ../Makefile.inc

SUBDIRS = doc po

compile: netconfig.so
	$(DO_RECURSIVE)

install:
	$(INSTALL) netconfig.so $(DESTDIR)$(fwlibdir)/netconfig.so
	mkdir -p $(DESTDIR)$(sysconfdir)/sysconfig/network
	ln -sf $(sbindir)/setup $(DESTDIR)$(sbindir)/netconfig
	$(DO_RECURSIVE)

clean:
	rm -f netconfig.so *.o
	$(DO_RECURSIVE)

%.so: %.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(basename $@).so -o \
		$(basename $@).so $(basename $@).o $(LDFLAGS)
