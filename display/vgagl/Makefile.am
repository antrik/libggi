AM_CFLAGS = -DPKG_NAME='"GGI:display:vgagl"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
vgagl_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg -lvga -lvgagl
vgagl_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = vgagl.la

vgagl_la_SOURCES = box.c fillscreen.c hline.c pixel.c visual.c vline.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS vgaglvis.h
