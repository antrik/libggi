INCLUDES = -I$(top_srcdir)/include $(extra_includes)
file_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
file_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = file.la

file_la_SOURCES = fileio.c mode.c ppm.c visual.c

sublibdir = $(libdir)/ggi/display

EXTRA_DIST = EXPSYMS
