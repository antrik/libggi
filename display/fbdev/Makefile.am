AM_CFLAGS = -DDEBUG_NAMESPACE='"libggi:display:fbdev"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
fbdev_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg
fbdev_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = fbdev.la

fbdev_la_SOURCES = visual.c mode.c pan.c color.c

sublibdir = $(libdir)/$(ggi_subdir)/display

fbdevconfdir = $(sysconfdir)/$(ggi_subdir)/targets
fbdevconf_DATA = fbdev.conf

EXTRA_DIST = EXPSYMS
