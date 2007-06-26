# Makefile.inc for frugalwareutils
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

VERSION = 0.7.3
MAJORVER = 0

DO_RECURSIVE = @ for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done
PO_DIR=~/darcs/translations/po
LANGS = $(shell ls $(PO_DIR)/frugalwareutils)

DESTDIR =
fwlibdir = $(libdir)/frugalware

# vim: ft=make
