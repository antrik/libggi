INCLUDES = -I$(top_srcdir)/include $(extra_includes)
linear_8_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
linear_8_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
		-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = linear_8.la

linear_8_la_SOURCES = box.c copybox.c crossblit.c gtext.c hline.c line.c \
	pixela.c pixel.c visual.c vline.c

sublibdir = $(libdir)/$(ggi_subdir)/default

EXTRA_DIST = EXPSYMS lin8lib.h
