bin_PROGRAMS = zhcon
zhcon_SOURCES = basefont.cpp big52gbdecoder.cpp big5decoder.cpp configfile.cpp console.cpp gb2big5decoder.cpp gbdecoder.cpp gbkdecoder.cpp graphdev.cpp hzdecoder.cpp jisdecoder.cpp kscdecoder.cpp main.cpp window.cpp winime.cpp zhcon.cpp overspotclient.cpp nativeinputserver.cpp inputclient.cpp inputmanager.cpp inputserver.cpp candilist.cpp uniconinputserver.cpp nativebarclient.cpp mouse.cpp encfilter.c iconv_string.c cmdline.c

EXTRA_DIST = basefont.h big52gbdecoder.h big5decoder.h configfile.h console.h debug.h fade.h gb2big5decoder.h gbdecoder.h gbkdecoder.h global.h graphdev.h hzdecoder.h jisdecoder.h keymap.h kscdecoder.h utypes.h window.h winime.h zhcon.h zhcon.conf ImmClient.h ImmComm.h ImmDefs.h ImmModule.h Phrase.h inputclient.h inputmanager.h inputserver.h nativeinputserver.h overspotclient.h candilist.h uniconinputserver.h nativebarclient.h logo.ansi logo.h mouse.h encfilter.h iconv_string.h cmdline.h opt.sh

zhcon_LDADD = display/libdisplay.a

SUBDIRS = display

install-exec-local:
	chmod 4755 $(bindir)/zhcon

install-data-local:
	$(mkinstalldirs) $(prefix)/etc
	$(INSTALL_DATA) $(srcdir)/zhcon.conf $(prefix)/etc/zhcon.conf

uninstall-local:
	-rm -f $(prefix)/etc/zhcon.conf

# set the include path found by configure
INCLUDES= $(all_includes)
 
# the library search path.
#zhcon_LDFLAGS = $(all_libraries) -L/usr/lib/unicon 

