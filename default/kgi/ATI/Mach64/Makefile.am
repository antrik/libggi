AM_CFLAGS = -DPKG_NAME='"LibGGI:default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
mach64_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
mach64_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
	-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = mach64.la

mach64_la_SOURCES = visual.c box.c line.c gc.c

sublibdir = $(libdir)/$(ggi_subdir)/default/kgi

EXTRA_DIST = EXPSYMS mach64_accel.h
