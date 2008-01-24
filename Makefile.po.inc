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
	cp /usr/bin/intltool-extract ./

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
	[ -d $(PO_DIR) ]
	for i in $(LANGS); \
	do \
		cp $(PO_DIR)/frugalwareutils/$$i/$(DOMAIN).po $$i.po; \
		if ! msgfmt -c --statistics -o $$i.gmo $$i.po; then \
			echo "ERROR: $(DOMAIN)/$$i.po would break your build!"; \
			exit 1; \
		fi; \
	done

# vim: ft=make
