AM_CFLAGS = -DPKG_NAME='"GGI:display:multi"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
multi_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg
multi_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = multi.la

multi_la_SOURCES = mode.c stubs.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS
