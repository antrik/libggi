AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.libkgi"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libkgilib_la_LIBADD = -lgalloc

noinst_LTLIBRARIES = libkgilib.la

libkgilib_la_SOURCES = visual.c gcops.c drawops.c

kgilibconfdir = $(sysconfdir)/$(ggi_subdir)/targets
kgilibconf_DATA = libkgi.conf

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in
