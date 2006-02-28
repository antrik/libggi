AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.default.fbdev.ati.mach64"'
INCLUDES = -I$(top_srcdir)/include
libmach64_la_LIBADD = 

noinst_LTLIBRARIES = libmach64.la

libmach64_la_SOURCES = box.c copybox.c crossblit.c gc.c gtext.c hline.c line.c \
	visual.c vline.c

EXTRA_DIST = EXPSYMS mach64.h ati_mach64.h

MAINTAINERCLEANFILES = Makefile.in

