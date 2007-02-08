SUBDIRS = @TESTSUBDIRS@
AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.regress"'
DIST_SUBDIRS = display.aa display.mansync display.palemu \
		display.quartz display.tile display.trueemu \
		display.x

INCLUDES = -I${top_srcdir}/include
LDADD = ${top_builddir}/ggi/libggi.la

input_LDADD = ${top_builddir}/ggi/libggi.la -lgii

EXTRA_PROGRAMS =
man_MANS =
noinst_PROGRAMS = clip2d init init-static input mapcolor mode pixfmt \
		triple-int

EXTRA_DIST = $(man_MANS) testsuite.inc.c clipdb.inc.c

MAINTAINERCLEANFILES = Makefile.in

init_static_LDFLAGS = -static
init_static_SOURCES = init.c
