AM_CFLAGS = -DDEBUG_NAMESPACE='"libggi:display:libkgi"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
kgilib_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg -lgalloc
kgilib_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = kgilib.la

kgilib_la_SOURCES = visual.c gcops.c drawops.c

sublibdir = $(libdir)/$(ggi_subdir)/display

kgilibconfdir = $(sysconfdir)/$(ggi_subdir)/targets
kgilibconf_DATA = libkgi.conf

EXTRA_DIST = EXPSYMS
