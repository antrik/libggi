AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libradeon_la_LIBADD =

noinst_LTLIBRARIES = libradeon.la

libradeon_la_SOURCES = visual.c box.c line.c gc.c gtext.c crossblit.c

EXTRA_DIST = EXPSYMS radeon_accel.h radeon_cce.h

MAINTAINERCLEANFILES = Makefile.in

