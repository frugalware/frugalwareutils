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

-include config.mak
-include Makefile.inc

SUBDIRS = doc libfwutil libfwdialog setup \
	  libfwnetconfig netconfig \
	  libfwtimeconfig timeconfig \
	  libfwraidconfig raidconfig \
	  libfwmouseconfig mouseconfig \
	  libfwxconfig xconfig \
	  libfwgrubconfig grubconfig \
	  libfwxwmconfig xwmconfig

compile: config.mak
	+$(DO_RECURSIVE)

configure: configure.ac
	cp /usr/share/automake/install-sh ./
	autoconf

config.mak: config.mak.in configure
	./configure

install:
	$(INSTALL) -d $(DESTDIR)$(sbindir)
	$(INSTALL) -d $(DESTDIR)$(fwlibdir)
	$(INSTALL) -d $(DESTDIR)$(libexecdir)
	$(INSTALL) -d $(DESTDIR)$(sysconfdir)/sysconfig/network
	$(INSTALL) -d $(DESTDIR)$(mandir)/man3
	$(INSTALL) -d $(DESTDIR)$(mandir)/man5
	$(INSTALL) -d $(DESTDIR)$(includedir)
	+$(DO_RECURSIVE)

clean:
	+$(DO_RECURSIVE)
	rm -rf autom4te.cache config.log config.mak config.status configure install-sh

dist:
	darcs changes >_darcs/current/Changelog
	darcs dist -d frugalwareutils-$(VERSION)
	gpg --comment "See http://ftp.frugalware.org/pub/README.GPG for info" \
		-ba -u 20F55619 frugalwareutils-$(VERSION).tar.gz
	mv frugalwareutils-$(VERSION).tar.gz{,.asc} ../
	rm _darcs/current/Changelog

release:
	darcs tag --checkpoint $(VERSION)
	$(MAKE) dist
