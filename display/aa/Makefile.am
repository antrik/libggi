AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.aa"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libaa_la_LIBADD = -laa

noinst_LTLIBRARIES = libaa.la

libaa_la_SOURCES = color.c giievents.c mode.c visual.c

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in
