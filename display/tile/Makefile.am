AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.tile"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
tile_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg
tile_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = tile.la

tile_la_SOURCES = color.c frames.c line.c mode.c stubs.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS
