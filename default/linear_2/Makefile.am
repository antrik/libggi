AM_CFLAGS = -DDEBUG_NAMESPACE='"libggi:default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
linear_2_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
linear_2_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
		-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = linear_2.la

linear_2_la_SOURCES = pixel.c pixela.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/default

EXTRA_DIST = EXPSYMS lin2lib.h
