AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.vnc"'
INCLUDES = -I$(top_srcdir)/include
libvnc_la_LIBADD = $(bsdsock_libs)

noinst_LTLIBRARIES = libvnc.la

libvnc_la_SOURCES = box.c buffer.c color.c gtext.c line.c pixel.c \
	mode.c visual.c \
	d3des.c d3des.h encoding.h rect.h rfb.c \
	copyrect.c corre.c hextile.c raw.c

if HAVE_ZLIB
libvnc_la_SOURCES += tight.c zlib.c zlibhex.c zrle.c
libvnc_la_LIBADD  += -lz
if HAVE_JPEG
libvnc_la_LIBADD  += -ljpeg
endif
endif

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in
