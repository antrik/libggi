AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.vnc"'
INCLUDES = -I$(top_srcdir)/include
libvnc_la_LIBADD =

noinst_LTLIBRARIES = libvnc.la

libvnc_la_SOURCES = d3des.c d3des.h mode.c rfb.c visual.c

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in
