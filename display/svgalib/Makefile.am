AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.svgalib"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
svgalib_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg -lvga
svgalib_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = svgalib.la

svgalib_la_SOURCES = box.c color.c frames.c hline.c mode.c pixel.c visual.c \
	vline.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

