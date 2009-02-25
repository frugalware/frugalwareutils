# Makefile.po.inc for frugalwareutils
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

LANG_CHARSETS = da_DK,ISO-8859-1 fr_FR,ISO-8859-1 hu_HU,ISO-8859-2

compile:
	for i in $(wildcard *.po); do \
		echo -n "$$i: "; \
		msgfmt -c --statistics -o $$i.mo $$i || exit 1; \
	done

install:
	for i in $(wildcard *.po); do \
		mkdir -p $(DESTDIR)$(localedir)/`basename $$i .po`/LC_MESSAGES/; \
		$(INSTALL) -m644 $$i.mo $(DESTDIR)$(localedir)/`basename $$i .po`/LC_MESSAGES/$(DOMAIN).mo; \
	done

intltool-extract:
	@if [ -e /usr/bin/intltool-extract ]; then \
		cp /usr/bin/intltool-extract ./; \
	else \
		echo 2>&1 "Install intltool first."; \
		exit 1; \
	fi

pot: intltool-extract
	intltool-update --pot --gettext-package=$(DOMAIN)

update-po: pot
	for lang in $(wildcard *.po); do \
		if msgmerge $$lang.po $(DOMAIN).pot -o $$lang.new.po; then \
			mv -f $$lang.new.po $$lang.po; \
		else \
			echo "msgmerge for $$cat failed!"; \
			rm -f $$lang.new.po; \
		fi; \
	done

%.po: $(DOMAIN).pot
	msginit -l $(basename $@)_$(shell echo $(basename $@)|tr [a-z] [A-Z])

clean:
	rm -f $(DOMAIN).pot *.mo *.po intltool-extract

prepare:
	@if [ -d $(PO_DIR) ]; then \
		for i in $(LANGS); \
		do \
			if [ -e $(PO_DIR)/frugalwareutils/$$i/$(DOMAIN).po ]; then \
				cp $(PO_DIR)/frugalwareutils/$$i/$(DOMAIN).po $$i.po; \
				charset=$$(echo $(LANG_CHARSETS)|sed 's/ /\n/g'|grep ^$$i|sed 's/.*,//'); \
				if [ -n "$$charset" ]; then \
					iconv -f UTF-8 -t $$charset < $$i.po > $$i.new.po; \
					sed "s/UTF-8/$$charset/" $$i.new.po > $$i.po; \
					rm $$i.new.po; \
				fi; \
				if ! msgfmt -c --statistics -o $$i.gmo $$i.po; then \
					echo "ERROR: $(DOMAIN)/$$i.po would break your build!"; \
					exit 1; \
				fi; \
			fi; \
		done; \
	else \
		echo "WARNING: $(PO_DIR) does not exist, skipping translations for $(DOMAIN)."; \
	fi

# vim: ft=make
