INCLUDES = -I$(top_srcdir)/include $(extra_includes)
quartz_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg

sublib_LTLIBRARIES = quartz.la

quartz_la_SOURCES = color.c gc.c mode.c visual.c
quartz_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
	-avoid-version -export-symbols $(srcdir)/EXPSYMS \
	-Xlinker -framework -Xlinker ApplicationServices \
	-Xlinker -framework -Xlinker Carbon


sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS quartz.h

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -no-cpp-precomp -I/System/Library/Frameworks -DDEBUG_NAMESPACE='"libggi.display.quartz"'

