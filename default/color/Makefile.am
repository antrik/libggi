INCLUDES = -I$(top_srcdir)/include $(extra_includes)
color_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg -lm
color_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = color.la

color_la_SOURCES = color.c pack.c gamma.c visual.c

sublibdir = $(libdir)/ggi/default

EXTRA_DIST = EXPSYMS color.h
