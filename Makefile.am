# ~/Makefile.am -- build and install the ioxx libraries

SUBDIRS			= src test

# dist_pkgdata_DATA       = README
CLEANFILES              = $(DIST_ARCHIVES)
MAINTAINERCLEANFILES    = missing install-sh configure Makefile.in aclocal.m4
MAINTAINERCLEANFILES   += config.guess config.sub ltmain.sh

aclocaldir              = $(datadir)/aclocal
dist_aclocal_DATA       = $(M4SOURCE)

htmldir                 = $(pkgdatadir)/html
dist_html_DATA          = $(HTMLDOC)
