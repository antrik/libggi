INCLUDES = -I$(top_srcdir)/include $(extra_includes)
directx_la_LIBADD = $(top_builddir)/ggi/libggi.la \
	-lddraw -lgdi32
directx_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
	-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = directx.la

directx_la_SOURCES = ddinit.c mode.c visual.c dxguid.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS ddinit.h
