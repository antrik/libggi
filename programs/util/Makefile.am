INCLUDES = -I${top_srcdir}/include ${extra_includes}
LDADD = ${top_builddir}/ggi/libggi.la

SUBDIRS = monitest

EXTRA_PROGRAMS = ggicalibrate ipaq_ggicalibrate $(EXTRAUTILS)
noinst_PROGRAMS = inputdump $(OPTUTILS)

inputdump_SOURCES = giik2str.c inputdump.c

inputdump_LDADD = ${top_builddir}/ggi/libggi.la -lgii
ipaq_ggicalibrate_LDADD = ${top_builddir}/ggi/libggi.la -lgii -lgg

EXTRA_DIST = giik2str.h
