AM_CFLAGS = -DDEBUG_NAMESPACE='"libggi:display:vgl"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
vgl_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg -lvgl
vgl_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = vgl.la

vgl_la_SOURCES = draw.c mode.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS
