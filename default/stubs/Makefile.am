AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libstubs_la_LIBADD =

noinst_LTLIBRARIES = libstubs.la

libstubs_la_SOURCES = box.c copybox.c crossblit.c fillscreen.c gtext.c hline.c \
	 line.c pixel.c visual.c vline.c 

EXTRA_DIST = EXPSYMS stublib.h

MAINTAINERCLEANFILES = Makefile.in

