AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.mansync"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
mansync_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
mansync_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = mansync.la

mansync_la_SOURCES = mansync.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS
