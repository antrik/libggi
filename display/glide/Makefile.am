AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.glide"'
INCLUDES = -I$(top_srcdir)/include
libglide_la_LIBADD = -lglide2x -lm

noinst_LTLIBRARIES = libglide.la

libglide_la_SOURCES = box.c fb.c gtext.c line.c mode.c pixel.c visual.c

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

