INCLUDES = -I${top_srcdir}/include ${extra_includes}
LDADD = ${top_builddir}/ggi/libggi.la

SUBDIRS = monitest

noinst_PROGRAMS = inputdump #ggicalibrate #tunemode

inputdump_SOURCES = giik2str.c inputdump.c #ggicalibrate.c #tunemode.c

inputdump_LDADD = ${top_builddir}/ggi/libggi.la -lgii
