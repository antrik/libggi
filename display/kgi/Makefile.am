INCLUDES = -I$(top_srcdir)/include $(extra_includes)
kgi_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
kgi_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = kgi.la

kgi_la_SOURCES = mode.c events.c visual.c

sublibdir = $(libdir)/ggi/display

EXTRA_DIST = EXPSYMS
