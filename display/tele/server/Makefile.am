AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.tele"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)

bin_PROGRAMS = ggiteleserver

ggiteleserver_SOURCES = ${top_builddir}/display/tele/server/libtele.c teleserver.c
ggiteleserver_LDADD = $(top_builddir)/ggi/libggi.la $(bsdsock_libs)

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = teleserver.c

${top_builddir}/display/tele/server/libtele.c: ${top_builddir}/display/tele/server/libtele.h
	rm -f ${top_builddir}/display/tele/server/libtele.c
	ln -s ${top_srcdir}/display/tele/libtele.c ${top_builddir}/display/tele/server/libtele.c

${top_builddir}/display/tele/server/libtele.h:
	rm -f ${top_builddir}/display/tele/server/libtele.h
	ln -s ${top_srcdir}/display/tele/libtele.h ${top_builddir}/display/tele/server/libtele.h

CLEANFILES = libtele.c libtele.h
MAINTAINERCLEANFILES = Makefile.in

