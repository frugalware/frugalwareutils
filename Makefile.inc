# Makefile.inc for frugalwareutils
#
# Copyright (C) 2006, 2007, 2008, 2009, 2010 Miklos Vajna
# <vmiklos@frugalware.org>
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

VERSION = 0.9.3
MAJORVER = 0

DO_RECURSIVE = @ for i in $(SUBDIRS); do $(MAKE) $(PRINT_DIR) -C $$i $@ || exit 1; done
PO_DIR=~/git/translations/po
LANGS = $(shell [ -d $(PO_DIR) ] && ls $(PO_DIR)/frugalwareutils)
CARCH ?= $(shell arch)

DESTDIR =
fwlibdir = $(libdir)/frugalware

# unless "make -w"
ifneq ($(findstring $(MAKEFLAGS),w),w)
PRINT_DIR = --no-print-directory
endif

# vim: ft=make
