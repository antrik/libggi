INCLUDES = -I$(top_srcdir)/include $(extra_includes)
memory_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg
memory_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = memory.la

memory_la_SOURCES = mode.c visual.c

sublibdir = $(libdir)/ggi/display

EXTRA_DIST = EXPSYMS
