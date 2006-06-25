compile:
	for i in $(LANGS); do \
		echo -n "$$i: "; \
		msgfmt -c --statistics -o $$i.mo $$i || exit 1; \
	done

install:
	for i in $(LANGS); do \
		mkdir -p $(DESTDIR)$(localedir)/`basename $$i .po`/LC_MESSAGES/; \
		$(INSTALL) -m644 $$i.mo $(DESTDIR)$(localedir)/`basename $$i .po`/LC_MESSAGES/$(DOMAIN).mo; \
	done

pot:
	intltool-update --pot --gettext-package=$(DOMAIN)

update-po: pot
	for lang in $(LANGS); do \
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
	rm -f $(DOMAIN).pot *.mo

# vim: ft=make
