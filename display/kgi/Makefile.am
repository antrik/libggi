AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.kgi"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
kgi_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
kgi_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS


sublib_LTLIBRARIES = kgi.la

kgi_la_SOURCES = mode.c visual.c kgi.c color.c accel.c

sublibdir = $(libdir)/$(ggi_subdir)/display

kgiconfdir = $(sysconfdir)/$(ggi_subdir)/targets
kgiconf_DATA = kgi.conf

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

