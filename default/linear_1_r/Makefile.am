INCLUDES = -I$(top_srcdir)/include $(extra_includes)
linear_1_r_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
linear_1_r_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = linear_1_r.la

linear_1_r_la_SOURCES = gtext.c pixel.c pixela.c visual.c

sublibdir = $(libdir)/ggi/default

EXTRA_DIST = EXPSYMS lin1rlib.h
