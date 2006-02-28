AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.vgl"'
INCLUDES = -I$(top_srcdir)/include
libvgl_la_LIBADD = -lvgl

noinst_LTLIBRARIES = libvgl.la

libvgl_la_SOURCES = draw.c mode.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

