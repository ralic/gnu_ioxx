# ~/Makefile.am -- build and install the ioxx library

ACLOCAL_AMFLAGS = -I build-aux
DISTCHECK_CONFIGURE_FLAGS = --without-doxygen

SUBDIRS = include test doc

CLEANFILES = $(DIST_ARCHIVES)

distclean-local:
	-rm -rf autom4te.cache

MAINTAINERCLEANFILES = \
  INSTALL \
  Makefile.in \
  aclocal.m4 \
  build-aux/config.guess \
  build-aux/config.h.in \
  build-aux/config.h.in~ \
  build-aux/config.sub \
  build-aux/depcomp \
  build-aux/install-sh \
  build-aux/ltmain.sh \
  build-aux/missing \
  build-aux/stamp-h1 \
  configure
