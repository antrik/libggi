INCLUDES = -I$(top_srcdir)/include $(extra_includes)
linear_4_r_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
linear_4_r_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
		-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = linear_4_r.la

linear_4_r_la_SOURCES = color.c copybox.c hline.c pixel.c pixela.c visual.c \
	vline.c

sublibdir = $(libdir)/ggi/default

EXTRA_DIST = EXPSYMS lin4rlib.h
