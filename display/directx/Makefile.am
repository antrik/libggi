INCLUDES = -I$(top_srcdir)/include $(extra_includes)
directx_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg -lgii \
	-lddraw -ldxguid -lgdi32
directx_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
	-avoid-version -export-symbols $(srcdir)/EXPSYMS
directxnt_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg -lgii \
	-lddraw -ldxguid -lgdi32
directxnt_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
	-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = directx.la directxnt.la

directx_la_SOURCES = ddcommon.c ddinit.c mode.c visual.c
directxnt_la_SOURCES = ddcommon.c ddinitnt.c mode.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS ddinit.h
