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

LANGS=hu

compile:

install:
	for i in $(LANGS); do \
		mkdir -p $(DESTDIR)$(localedir)/`basename $$i .po`/LC_MESSAGES/; \
		msgfmt -o $(DESTDIR)$(localedir)/`basename $$i .po`/LC_MESSAGES/netconfig.mo $$i; \
	done

pot:
	intltool-update --pot --gettext-package=netconfig

update-po: pot
	for lang in $(LANGS); do \
		if msgmerge $$lang.po netconfig.pot -o $$lang.new.po; then \
			mv -f $$lang.new.po $$lang.po; \
		else \
			echo "msgmerge for $$cat failed!"; \
			rm -f $$lang.new.po; \
		fi; \
	done

clean:
	rm -f netconfig.pot
