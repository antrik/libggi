INCLUDES = -I${top_srcdir}/include ${extra_includes}
LDADD = ${top_builddir}/ggi/libggi.la ${THREADLIBS}

SUBDIRS = monitest

EXTRA_PROGRAMS = tunemode ggicalibrate ipaq_ggicalibrate
noinst_PROGRAMS = inputdump $(OPTUTILS)

inputdump_SOURCES = giik2str.c inputdump.c

#inputdump_LDADD = ${top_builddir}/ggi/libggi.la ${THREADLIBS}
#ipaq_ggicalibrate_LDADD = ${top_builddir}/ggi/libggi.la -lgii -lgg ${THREADLIBS}

EXTRA_DIST = giik2str.h
