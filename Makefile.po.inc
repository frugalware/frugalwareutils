compile:

install:
	for i in $(LANGS); do \
		mkdir -p $(DESTDIR)$(localedir)/`basename $$i .po`/LC_MESSAGES/; \
		msgfmt -o $(DESTDIR)$(localedir)/`basename $$i .po`/LC_MESSAGES/$(DOMAIN).mo $$i; \
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

clean:
	rm -f $(DOMAIN).pot

# vim: ft=make
