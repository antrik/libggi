AM_CFLAGS = -DDEBUG_NAMESPACE='"libggi:display:linvtsw"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
linvtsw_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
linvtsw_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = linvtsw.la

# We want the getpgid() prototype
AM_CPPFLAGS = -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED

linvtsw_la_SOURCES = visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS
