SUBDIRS = @TESTSUBDIRS@
DIST_SUBDIRS = display.aa display.mansync display.palemu \
		display.tile display.trueemu display.x

INCLUDES = -I${top_srcdir}/include ${extra_includes}
LDADD = ${top_builddir}/ggi/libggi.la

EXTRA_PROGRAMS =
man_MANS =
noinst_PROGRAMS = clip2d init input mode pixfmt triple-int

EXTRA_DIST = $(man_MANS) testsuite.inc.c clipdb.inc.c
