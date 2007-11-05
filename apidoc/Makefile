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

-include ../Makefile.inc
-include ../config.mak

ifneq ($(asciidoc),)
compile: frugalwareutils.3
else
compile:
endif
ifneq ($(doxygen),)
	cd .. && doxygen Doxyfile
	rm -rf man3/_*
endif

install:
ifneq ($(doxygen),)
	for i in man3/*; do \
		$(INSTALL) -m644 $$i $(DESTDIR)$(mandir)/man3/; \
	done
endif
ifneq ($(asciidoc),)
	$(INSTALL) -m644 frugalwareutils.3 $(DESTDIR)$(mandir)/man3/
endif

clean:
	rm -rf man3 frugalwareutils.3 *.xml

prepare:

frugalwareutils.3:
	asciidoc -d manpage -b docbook frugalwareutils.txt
	sed -i '/<!DOCTYPE/s|\("http[^"].*"\)|"file:///usr/share/sgml/docbook/dtd/xml-dtd-4.2/docbookx.dtd"|' \
		frugalwareutils.xml
	xsltproc -o frugalwareutils.3 --nonet --path /usr/share/sgml/docbook/dtd/xml-dtd-4.2 \
		/etc/asciidoc/docbook-xsl/manpage.xsl frugalwareutils.xml
	sed -i 's/\\(bu/*/' frugalwareutils.3
