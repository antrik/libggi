AM_CFLAGS = -DDEBUG_NAMESPACE='"libggi:default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
iplanar_2p_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
iplanar_2p_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
	-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = iplanar_2p.la

iplanar_2p_la_SOURCES = pixel.c pixela.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/default

EXTRA_DIST = EXPSYMS ipl2lib.h
