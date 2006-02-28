AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.suidkgi"' -DLINUX_SUID
INCLUDES = -I$(top_srcdir)/include
libsuidkgi_la_LIBADD = suidkgi.o

noinst_LTLIBRARIES = libsuidkgi.la

libsuidkgi_la_SOURCES = mode.c suidbridge.c visual.c

libsublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS clut.inc font.inc \
	ibm-cp437.08x08.inc ibm-cp437.08x14.inc ibm-cp437.09x16.inc \
	ibm-cp437.inc suidhook.h

MAINTAINERCLEANFILES = Makefile.in
