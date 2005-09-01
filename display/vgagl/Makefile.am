AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.vgagl"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libvgagl_la_LIBADD = -lvga -lvgagl

noinst_LTLIBRARIES = libvgagl.la

libvgagl_la_SOURCES = box.c fillscreen.c hline.c pixel.c visual.c vline.c

EXTRA_DIST = EXPSYMS vgaglvis.h

MAINTAINERCLEANFILES = Makefile.in

