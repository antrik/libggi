AM_CFLAGS = -DDEBUG_NAMESPACE='"libggi:default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
linear_24_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
linear_24_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
		-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = linear_24.la

linear_24_la_SOURCES = box.c crossblit.c copybox.c hline.c pixel.c pixela.c \
	visual.c vline.c

sublibdir = $(libdir)/$(ggi_subdir)/default

EXTRA_DIST = EXPSYMS lin24lib.h
