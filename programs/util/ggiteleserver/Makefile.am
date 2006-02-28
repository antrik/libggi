AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.tele"'
INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = ggiteleserver

ggiteleserver_SOURCES = ${top_builddir}/programs/util/ggiteleserver/libtele.c teleserver.c
ggiteleserver_LDADD = $(top_builddir)/ggi/libggi.la $(bsdsock_libs)

EXTRA_DIST = teleserver.c

${top_builddir}/programs/util/ggiteleserver/libtele.c: ${top_builddir}/programs/util/ggiteleserver/libtele.h
	rm -f ${top_builddir}/programs/util/ggiteleserver/libtele.c
	ln -s ${top_srcdir}/display/tele/libtele.c ${top_builddir}/programs/util/ggiteleserver/libtele.c

${top_builddir}/programs/util/ggiteleserver/libtele.h:
	rm -f ${top_builddir}/programs/util/ggiteleserver/libtele.h
	ln -s ${top_srcdir}/display/tele/libtele.h ${top_builddir}/programs/util/ggiteleserver/libtele.h

CLEANFILES = libtele.c libtele.h
MAINTAINERCLEANFILES = Makefile.in
