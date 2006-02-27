# We want the getpgid() prototype
AM_CPPFLAGS += -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED -DDEBUG_NAMESPACE='"libggi.display.linvtsw"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
liblinvtsw_la_LIBADD =

noinst_LTLIBRARIES = liblinvtsw.la

liblinvtsw_la_SOURCES = visual.c

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

