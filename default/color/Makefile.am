AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.default.color"'
INCLUDES = -I$(top_srcdir)/include
libcolor_la_LIBADD = -lm

noinst_LTLIBRARIES = libcolor.la

libcolor_la_SOURCES = color.c pack.c gamma.c visual.c

EXTRA_DIST = EXPSYMS color.h

MAINTAINERCLEANFILES = Makefile.in

