# Makefile for frugalwareutils
#
# Copyright (C) 2006, 2007, 2008, 2009 Miklos Vajna <vmiklos@frugalware.org>
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

SUBDIRS = doc apidoc libfwutil libfwdialog setup netconfigd \
	  libfwnetconfig netconfig \
	  libfwtimeconfig timeconfig \
	  libfwraidconfig raidconfig \
	  libfwmouseconfig mouseconfig \
	  libfwxwmconfig xwmconfig

ifneq ($(CARCH),ppc)
SUBDIRS += libfwgrubconfig grubconfig
endif

ifeq ($(CARCH),ppc)
SUBDIRS += libfwyabootcfg yabootcfg
endif

compile: config.mak
	+$(DO_RECURSIVE)

prepare: configure.ac
	@if [ -e /usr/share/automake/install-sh ]; then \
		cp /usr/share/automake/install-sh ./; \
	else \
		echo 2>&1 "Install automake first."; \
		exit 1; \
	fi
	@if [ -e /usr/share/aclocal/pkg.m4 ]; then \
		cp /usr/share/aclocal/pkg.m4 aclocal.m4; \
	else \
		echo 2>&1 "Install pkgconfig first."; \
		exit 1; \
	fi
	autoconf
	+$(DO_RECURSIVE)

install:
	$(INSTALL) -d $(DESTDIR)$(sbindir)
	$(INSTALL) -d $(DESTDIR)$(fwlibdir)
	$(INSTALL) -d $(DESTDIR)$(libexecdir)
	$(INSTALL) -d $(DESTDIR)$(sysconfdir)/sysconfig/network
	$(INSTALL) -d $(DESTDIR)$(sysconfdir)/systemd/system/sysinit.target.wants
	$(INSTALL) -d $(DESTDIR)$(libdir)/systemd/system
	$(INSTALL) -d $(DESTDIR)/usr/lib/pm-utils/sleep.d
	$(INSTALL) -m644 netconfig/netconfig.service $(DESTDIR)$(libdir)/systemd/system/netconfig.service
	$(INSTALL) -m644 netconfig/netconfig-wait-online.service $(DESTDIR)$(libdir)/systemd/system/netconfig-wait-online.service
	$(INSTALL) -m755 netconfig/netconfig.sleep $(DESTDIR)/usr/lib/pm-utils/sleep.d/55netconfig
	$(INSTALL) -d $(DESTDIR)$(mandir)/man1
	$(INSTALL) -d $(DESTDIR)$(mandir)/man3
	$(INSTALL) -d $(DESTDIR)$(includedir)
	+$(DO_RECURSIVE)

clean:
	+$(DO_RECURSIVE)
	rm -rf autom4te.cache config.log config.mak config.status

distclean: clean
	rm -f configure install-sh aclocal.m4 */po/*.gmo

dist:
	git archive --format=tar --prefix=frugalwareutils-$(VERSION)/ HEAD | tar xf -
	git log --no-merges |git name-rev --tags --stdin > frugalwareutils-$(VERSION)/Changelog
	make -C frugalwareutils-$(VERSION) prepare
	tar czf frugalwareutils-$(VERSION).tar.gz frugalwareutils-$(VERSION)
	rm -rf frugalwareutils-$(VERSION)

release:
	git tag -l |grep -q $(VERSION) || dg tag $(VERSION)
	$(MAKE) dist
	gpg --comment "See http://ftp.frugalware.org/pub/README.GPG for info" \
		-ba -u 20F55619 frugalwareutils-$(VERSION).tar.gz
	mv frugalwareutils-$(VERSION).tar.gz{,.asc} ../

update-po:
	for i in `find . -type d -name po`; \
	do \
		$(MAKE) -C $$i update-po; \
	done

pot:
	@for i in `find . -type d -name po`; \
	do \
		$(MAKE) $(PRINT_DIR) -C $$i pot || exit 1; \
	done

doc: ../HEADER.html ../Changelog

../HEADER.html: README
	asciidoc -a toc -a numbered -a sectids -o ../HEADER.html README

../Changelog: .git/refs/heads/master
	git log --no-merges |git name-rev --tags --stdin >../Changelog
