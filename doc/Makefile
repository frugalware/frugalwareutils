# Makefile for frugalwareutils
#
# Copyright (C) 2007 Miklos Vajna <vmiklos@frugalware.org>
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
-include ../config.mak

DOCS = $(wildcard *.txt)
MANS = $(subst .txt,.1,$(DOCS))

ifneq ($(asciidoc),)
compile: $(MANS)
else
compile:
endif

install:
ifneq ($(asciidoc),)
	$(INSTALL) -m644 $(MANS) $(DESTDIR)$(mandir)/man1
endif

clean:
	rm -f *.1

prepare:

%.html: %.txt
	asciidoc $^

%.1: %.txt
	a2x -f manpage $^
