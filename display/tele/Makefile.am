AM_CFLAGS = -DDEBUG_NAMESPACE='"libggi:display:tele"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
tele_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg $(bsdsock_libs)
tele_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = tele.la

tele_la_SOURCES = box.c color.c draw.c events.c mode.c text.c visual.c \
	libtele.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS libtele.h

SUBDIRS = server
