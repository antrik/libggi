AM_CFLAGS = -DDEBUG_NAMESPACE='"libggi:display:wsfb"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
wsfb_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg
wsfb_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = wsfb.la

wsfb_la_SOURCES = mode.c visual.c #draw.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS
