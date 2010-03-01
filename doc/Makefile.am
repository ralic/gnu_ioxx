# Build the ioxx library.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

EXTRA_DIST = html

html-local:
	$(DOXYGEN)

install-data-local:	html-local
	if [ -d html ]; then \
	  $(MKDIR_P) $(DESTDIR)$(htmldir)/html; \
	  $(INSTALL_DATA) html/* $(DESTDIR)$(htmldir)/html; \
	elif [ -d "${top_srcdir}/doc/html" ]; then \
	  $(MKDIR_P) $(DESTDIR)$(htmldir)/html; \
	  $(INSTALL_DATA) ${top_srcdir}/doc/html/* $(DESTDIR)$(htmldir)/html; \
	fi

uninstall-local:
	-rm -rf $(DESTDIR)$(htmldir)/html

maintainerclean-local:
	-rm -rf html

MAINTAINERCLEANFILES = \
  Makefile.in
