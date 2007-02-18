AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.aa"'
INCLUDES = -I$(top_srcdir)/include

# Use libgaa name to avoid conflict with libaa.la provided
# by libaa. The conflict confuses libtool.
libgaa_la_LIBADD = -laa

noinst_LTLIBRARIES = libgaa.la

libgaa_la_SOURCES = color.c mode.c listener.c visual.c

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in
