SUBDIRS = include display default ggi display-shared default-shared extensions programs doc m4 dist

ggiconfdatadir = $(sysconfdir)/$(ggi_subdir)
ggiconfdata_DATA = $(ggi_conffile)

noinst_PROGRAMS = patchlib

DISTCLEANFILES = libggi.conf config.lt

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.h.in \
	configure

EXTRA_DIST = ChangeLog.1999 ChangeLog FAQ INSTALL.autoconf \
	autogen.sh checkversion.sh
