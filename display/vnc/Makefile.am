AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.vnc"'
INCLUDES = -I$(top_srcdir)/include
libvnc_la_LIBADD = $(bsdsock_libs)

noinst_LTLIBRARIES = libvnc.la

libvnc_la_SOURCES = box.c buffer.c color.c gtext.c line.c pixel.c \
	mode.c visual.c \
	encoding.h rect.h rfb.c copyrect.c corre.c hextile.c raw.c rre.c

if HAVE_ZLIB
libvnc_la_SOURCES += tight.c zlib.c zlibhex.c zrle.c
libvnc_la_LIBADD  += -lz
if HAVE_JPEG
if HAVE_TURBOJPEG
libvnc_la_LIBADD  += -lturbojpeg
else
if HAVE_JPEGLIB
libvnc_la_LIBADD  += -ljpeg
endif
endif
endif
endif

if HAVE_OPENSSL
libvnc_la_SOURCES += vencrypt.c
AM_CFLAGS         += $(openssl_CFLAGS)
libvnc_la_LIBADD  += $(openssl_LIBS)
else
libvnc_la_SOURCES += d3des.c d3des.h
endif

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in
