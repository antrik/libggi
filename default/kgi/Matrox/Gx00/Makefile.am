INCLUDES = -I$(top_srcdir)/include $(extra_includes)
Gx00_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
Gx00_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
	-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = Gx00.la

Gx00_la_SOURCES = visual.c box.c line.c gc.c gtext.c

sublibdir = $(libdir)/$(ggi_subdir)/default/kgi

EXTRA_DIST = EXPSYMS Gx00.h Gx00_accel.h
