AM_CFLAGS = -DDEBUG_NAMESPACE='"libggi:default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
linear_16_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
linear_16_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
		-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = linear_16.la

linear_16_la_SOURCES = box.c copybox.c crossblit.c hline.c line.c pixel.c \
	pixela.c visual.c vline.c

sublibdir = $(libdir)/$(ggi_subdir)/default

EXTRA_DIST = EXPSYMS lin16lib.h
