AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.vcsa"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
vcsa_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg
vcsa_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = vcsa.la

vcsa_la_SOURCES = color.c draw.c mode.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

