AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.sub"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
sub_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
sub_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = sub.la

sub_la_SOURCES = mode.c stubs.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS
