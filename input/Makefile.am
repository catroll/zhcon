EXTRA_DIST = *.mb

install-data-local:
	$(mkinstalldirs) $(libdir)/zhcon/input/
	$(INSTALL_DATA) $(srcdir)/*.mb $(libdir)/zhcon/input/

uninstall-local:
	-rm -f $(libdir)/zhcon/input/*.mb
