AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.auto"'
EXTRA_DIST =

INCLUDES = -I$(top_srcdir)/include $(extra_includes)
auto_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
auto_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = auto.la

include $(srcdir)/os/os.am
include $(srcdir)/target/target.am

auto_la_SOURCES = probe.c visual.c $(os_src) $(target_src)

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST += EXPSYMS target/target.h

MAINTAINERCLEANFILES = Makefile.in

