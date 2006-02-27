AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.tele"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libtele_la_LIBADD = $(bsdsock_libs)

noinst_LTLIBRARIES = libtele.la

libtele_la_SOURCES = box.c color.c draw.c events.c mode.c text.c visual.c \
	libtele.c

EXTRA_DIST = EXPSYMS libtele.h

MAINTAINERCLEANFILES = Makefile.in
