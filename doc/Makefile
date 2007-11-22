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

LANGS = $(shell grep '\[po4a_langs\]' po4a.cfg |sed 's/\[po4a_langs\] //')

CONVERT = \
	  if echo $(1)|grep -q /; then \
	  iconv -t $(shell locale -a |grep "$(shell dirname $(1))_.*\."|sed -n 's/.*\.//;1 p') -f UTF8 < $(1) > tmp; mv tmp $(1); \
	  fi

ifneq ($(asciidoc),)
compile: $(MANS) po4a localized_mans
else
compile:
endif

po4a:
ifneq ($(po4a),)
	po4a -k 0 po4a.cfg
endif

ifneq ($(po4a),)
localized_mans: $(shell for i in $(LANGS); do for j in $(MANS); do echo $$i/$$j; done; done)
install-man: install-man1
else
localized_mans:
endif

install: install-man
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

install-man1:
	for i in $(LANGS); do \
		$(INSTALL) -D -m755 $(DESTDIR)$(mandir)/$$i/man1; \
		for j in $(MANS); do \
			$(INSTALL) -m644 $$i/$$j "$(DESTDIR)$(mandir)/$$i/man1/$$j"; \
		done; \
	done

pot:
	po4a -k 0 po4a.cfg
	rm -rf po/*.po $(LANGS)
