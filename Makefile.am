AUTOMAKE_OPTIONS = 1.4 foreign

SUBDIRS = include ggi display default extensions programs doc m4 dist

ggiconfdatadir = $(sysconfdir)/$(ggi_subdir)
ggiconfdata_DATA = libggi.conf

noinst_PROGRAMS = patchlib

DISTCLEANFILES = libggi.conf
MAINTAINERCLEANFILES = Makefile.in acinclude.m4 aclocal.m4 config.h.in \
	configure genlibtool

EXTRA_DIST = ChangeLog.1999 ChangeLog FAQ INSTALL.autoconf \
	autogen.sh checkversion.sh genlibtool genlibtool.in
