INCLUDES = -I$(top_srcdir)/include $(extra_includes)
linear_32_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
linear_32_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
		-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = linear_32.la

linear_32_la_SOURCES = box.c copybox.c crossblit.c hline.c line.c pixel.c \
	pixela.c visual.c vline.c

sublibdir = $(libdir)/ggi/default

EXTRA_DIST = EXPSYMS lin32lib.h
