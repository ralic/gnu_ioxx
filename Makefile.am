# ~/Makefile.am -- build and install the ioxx library

ACLOCAL_AMFLAGS = -I build-aux

SUBDIRS = include src test

CLEANFILES = $(DIST_ARCHIVES)

distclean-local:
	-rm -rf autom4te.cache

MAINTAINERCLEANFILES = \
  Makefile.in \
  aclocal.m4 \
  build-aux/config.guess \
  build-aux/config.sub \
  build-aux/depcomp \
  build-aux/install-sh \
  build-aux/ltmain.sh \
  build-aux/missing \
  configure
