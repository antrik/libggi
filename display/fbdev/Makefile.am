AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.fbdev"'
INCLUDES = -I$(top_srcdir)/include
libfbdev_la_LIBADD =

noinst_LTLIBRARIES = libfbdev.la

libfbdev_la_SOURCES = color.c mode.c pan.c visual.c

fbdevconfdir = $(sysconfdir)/$(ggi_subdir)/targets
fbdevconf_DATA = fbdev.conf

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

