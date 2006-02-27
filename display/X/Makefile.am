AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.X"'
SUBDIRS = helper
DIST_SUBDIRS = helper
INCLUDES = -I$(top_srcdir)/include $(extra_includes) $(X_CFLAGS)
libx_la_LIBADD = $(X_LIBS) -lX11 $(X_EXTRA_LIBS)

noinst_LTLIBRARIES = libx.la

libx_la_SOURCES = pixel.c hline.c vline.c box.c line.c fillscreen.c gtext.c \
	       visual.c mode.c buffer.c color.c misc.c 

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

