AM_CFLAGS = -DDEBUG_NAMESPACE='"libggi:display:lcd823"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
lcd823_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg
lcd823_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = lcd823.la

lcd823_la_SOURCES = visual.c mode.c color.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS
