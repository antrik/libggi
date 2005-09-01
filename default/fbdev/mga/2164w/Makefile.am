AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libm2164w_la_LIBADD =

noinst_LTLIBRARIES = libm2164w.la

libm2164w_la_SOURCES = box.c copybox.c crossblit.c gtext.c hline.c line.c \
	visual.c vline.c

EXTRA_DIST = EXPSYMS m2164w.h mmio.h regs.h

MAINTAINERCLEANFILES = Makefile.in

