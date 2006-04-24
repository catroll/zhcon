EXTRA_DIST = *.bpsf

install-data-local:
	$(mkinstalldirs) $(libdir)/zhcon/font/
	$(INSTALL_DATA) $(srcdir)/*.bpsf $(libdir)/zhcon/font/

uninstall-local:
	-rm -f $(libdir)/zhcon/font/*.bpsf
