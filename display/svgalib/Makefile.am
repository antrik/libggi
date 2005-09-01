AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.svgalib"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libsvgalib_la_LIBADD = -lvga

noinst_LTLIBRARIES = libsvgalib.la

libsvgalib_la_SOURCES = box.c color.c frames.c hline.c mode.c pixel.c visual.c \
	vline.c

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in
