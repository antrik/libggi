AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.kgi"'
INCLUDES = -I$(top_srcdir)/include
libkgi_la_LIBADD =

noinst_LTLIBRARIES = libkgi.la

libkgi_la_SOURCES = mode.c visual.c color.c accel.c
if OS_GNU
libkgi_la_SOURCES += kgi_hurd.c
nodist_libkgi_la_SOURCES = kgiUser.c
BUILT_SOURCES = kgiUser.h
CLEANFILES = kgiUser.c kgiUser.h

%User.h %User.c:
	mig -cc '$(CPP) -x c $(CPPFLAGS) -' -imacros kgi/hurd/$*mutations.h -header kgiUser.h -user kgiUser.c -server /dev/null /dev/null <<<"#include <kgi/hurd/$*.defs>"
else
libkgi_la_SOURCES += kgi.c
endif

kgiconfdir = $(sysconfdir)/$(ggi_subdir)/targets
kgiconf_DATA = kgi.conf

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

