INCLUDES = -I$(top_srcdir)/include $(extra_includes) -DLINUX_SUID
suidkgi_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg suidkgi.o
suidkgi_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = suidkgi.la

suidkgi_la_SOURCES = mode.c suidbridge.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS clut.inc font.inc \
	ibm-cp437.08x08.inc ibm-cp437.08x14.inc ibm-cp437.09x16.inc \
	ibm-cp437.inc suidhook.h
