AUTOMAKE_OPTIONS = 1.4 foreign

SUBDIRS = include display default ggi display-shared default-shared extensions programs doc m4 dist

ggiconfdatadir = $(sysconfdir)/$(ggi_subdir)
ggiconfdata_DATA = libggi.conf

noinst_PROGRAMS = patchlib

DISTCLEANFILES = libggi.conf config.lt

MAINTAINERCLEANFILES = Makefile.in acinclude.m4 aclocal.m4 config.h.in \
	configure

EXTRA_DIST = ChangeLog.1999 ChangeLog FAQ INSTALL.autoconf \
	autogen.sh checkversion.sh
