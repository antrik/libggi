INCLUDES = -I${top_srcdir}/include ${extra_includes}
LDADD = ${top_builddir}/ggi/libggi.la

SUBDIRS = monitest

noinst_PROGRAMS = inputdump #tunemode

inputdump_SOURCES = giik2str.c inputdump.c

inputdump_LDADD = ${top_builddir}/ggi/libggi.la -lgii
