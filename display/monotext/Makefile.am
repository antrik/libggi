AM_CFLAGS = -DPKG_NAME='"GGI:display:monotext"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
monotext_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg -lm
monotext_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = monotext.la

monotext_la_SOURCES = color.c draw.c mode.c monotext.c text.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS font_data.h
