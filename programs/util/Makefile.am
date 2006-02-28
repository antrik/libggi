INCLUDES = -I${top_srcdir}/include
LDADD = ${top_builddir}/ggi/libggi.la

SUBDIRS = monitest ggiteleserver

EXTRA_PROGRAMS = tunemode ggicalibrate ipaq_ggicalibrate
noinst_PROGRAMS = inputdump $(OPTUTILS)

inputdump_SOURCES = giik2str.c inputdump.c

#inputdump_LDADD = ${top_builddir}/ggi/libggi.la
#ipaq_ggicalibrate_LDADD = ${top_builddir}/ggi/libggi.la

EXTRA_DIST = giik2str.h

MAINTAINERCLEANFILES = Makefile.in
