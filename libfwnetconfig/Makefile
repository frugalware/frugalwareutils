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

-include ../config.mak
CFLAGS = $(LIB_CFLAGS)
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

# fwutil
CFLAGS += -I../libfwutil -I/usr/include/pci
LDFLAGS += -L../libfwutil -lfwutil -lpci -lz

#SUBDIRS = po
LIBNAME = netconfig
LIBOBJS = pci.o

-include ../Makefile.inc
-include ../Makefile.lib.inc
