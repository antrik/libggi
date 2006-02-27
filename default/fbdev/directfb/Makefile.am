AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.default.fbdev.directfb"'
SUBDIRS = core

INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libdirectfb_la_LIBADD =

libdirectfbglobal_la_LIBADD = 

noinst_LTLIBRARIES = libdirectfb.la libdirectfbglobal.la

libdirectfb_la_SOURCES = box.c copybox.c crossblit.c gtext.c hline.c line.c \
	visual.c vline.c

libdirectfbglobal_la_SOURCES = directfbglobal.c

EXTRA_DIST = EXPSYMS EXPSYMS.global ggidirectfb.h

DIST_SUBDIRS = core

MAINTAINERCLEANFILES = Makefile.in

