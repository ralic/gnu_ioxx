# Build the ioxx library.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

ACLOCAL_AMFLAGS = -I build-aux
DISTCHECK_CONFIGURE_FLAGS = --without-doxygen

SUBDIRS = include test doc

CLEANFILES = $(DIST_ARCHIVES)

DISTCLEANFILES = _configs.sed

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

distclean-local:
	-rm -rf autom4te.cache

dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version
	if test -d .git; then							\
	  $(top_srcdir)/build-aux/gitlog-to-changelog >$(distdir)/cl-t master;	\
	  rm -f $(distdir)/ChangeLog;						\
	  mv $(distdir)/cl-t $(distdir)/ChangeLog;				\
	fi
