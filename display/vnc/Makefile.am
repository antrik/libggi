AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.vnc"'
INCLUDES = -I$(top_srcdir)/include
libvnc_la_LIBADD =

noinst_LTLIBRARIES = libvnc.la

libvnc_la_SOURCES = box.c buffer.c color.c gtext.c line.c pixel.c \
	mode.c visual.c \
	d3des.c d3des.h encoding.h raw.c rect.h rfb.c

if HAVE_ZLIB
libvnc_la_LIBADD  += -lz
libvnc_la_SOURCES += zlib.c zrle.c
endif

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in