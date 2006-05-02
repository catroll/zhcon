#docs are not installed automatically now.

EXTRA_DIST = zhcon.1.in poem.big5 poem.gb poem.gb.utf8 bpsf.txt manual.tex manual.dvi manual.ps README.html default.css html.tar.gz

install-data-local:
	$(mkinstalldirs) $(mandir)/man1/
	$(INSTALL_DATA) $(top_builddir)/doc/zhcon.1 $(mandir)/man1/zhcon.1

uninstall-local:
	-rm -f $(mandir)/man1/zhcon.1
