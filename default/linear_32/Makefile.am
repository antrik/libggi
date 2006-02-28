AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.default.linear_32"'
INCLUDES = -I$(top_srcdir)/include
liblinear_32_la_LIBADD =

noinst_LTLIBRARIES = liblinear_32.la

liblinear_32_la_SOURCES = box.c copybox.c crossblit.c hline.c line.c pixel.c \
	pixela.c visual.c vline.c

EXTRA_DIST = EXPSYMS lin32lib.h

MAINTAINERCLEANFILES = Makefile.in

