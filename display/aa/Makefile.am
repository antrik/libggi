AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.aa"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
aa_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg -laa
aa_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = aa.la

aa_la_SOURCES = color.c giievents.c mode.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS
