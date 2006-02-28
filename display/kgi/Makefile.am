AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.kgi"'
INCLUDES = -I$(top_srcdir)/include
libkgi_la_LIBADD =

noinst_LTLIBRARIES = libkgi.la

libkgi_la_SOURCES = mode.c visual.c kgi.c color.c accel.c

kgiconfdir = $(sysconfdir)/$(ggi_subdir)/targets
kgiconf_DATA = kgi.conf

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

