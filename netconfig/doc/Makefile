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

-include ../../Makefile.inc

SUBDIRS = po

LANGS=hu

compile:
	po4a po4a.cfg

install:
	$(INSTALL) -m644 netconfig.5 $(DESTDIR)$(mandir)/man5/netconfig.5
	for lang in $(LANGS); do \
		mkdir -p $(DESTDIR)$(mandir)/$$lang/man5/; \
		$(INSTALL) -m644 $$lang/netconfig.5 $(DESTDIR)$(mandir)/$$lang/man5/netconfig.5; \
	done

clean:
	rm -rf $(LANGS)
	$(DO_RECURSIVE)
