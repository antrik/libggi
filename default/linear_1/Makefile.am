INCLUDES = -I$(top_srcdir)/include $(extra_includes)
linear_1_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
linear_1_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = linear_1.la

linear_1_la_SOURCES = gtext.c hline.c pixel.c pixela.c visual.c vline.c

sublibdir = $(libdir)/$(ggi_subdir)/default

EXTRA_DIST = EXPSYMS lin1lib.h
