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

-include Makefile.inc

SUBDIRS = libfwutil libfwdialog libfwnetconfig netconfig \
	  libfwtimeconfig timeconfig

compile:
	$(DO_RECURSIVE)

install: compile
	$(INSTALL) -d $(DESTDIR)$(sbindir)
	$(INSTALL) -d $(DESTDIR)$(libdir)
	$(INSTALL) -d $(DESTDIR)$(usrsbindir)
	$(INSTALL) -d $(DESTDIR)$(sysconfdir)
	$(INSTALL) -d $(DESTDIR)$(mandir)/man5
	$(DO_RECURSIVE)

clean:
	$(DO_RECURSIVE)

dist:
	darcs changes >_darcs/current/Changelog
	darcs dist -d frugalwareutils-$(VERSION)
	rm _darcs/current/Changelog
	mv frugalwareutils-$(VERSION).tar.gz ../
