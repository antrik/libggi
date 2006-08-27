AM_CPPFLAGS += -no-cpp-precomp -I/System/Library/Frameworks -DDEBUG_NAMESPACE='"libggi.display.quartz"'
INCLUDES = -I$(top_srcdir)/include
libquartz_la_LIBADD =

noinst_LTLIBRARIES = libquartz.la

libquartz_la_SOURCES = color.c gc.c listener.c mode.c visual.c
libquartz_la_LDFLAGS = -framework ApplicationServices \
		-framework Carbon

EXTRA_DIST = EXPSYMS quartz.h

MAINTAINERCLEANFILES = Makefile.in
