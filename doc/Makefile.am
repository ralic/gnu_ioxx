# ~/doc/Makefile.am

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
