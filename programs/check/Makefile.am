INCLUDES = -I${top_srcdir}/include
LDADD = ${top_builddir}/ggi/libggi.la

EXTRA_PROGRAMS = showaccel2 speed
noinst_PROGRAMS = checkmode showaccel findleaks \
	consistency structs cbconsist pointer palette \
	$(OPTCHECKS)

# -lgg is needed for ggUSleep() and ggCurTime()
showaccel_LDADD =  ${top_builddir}/ggi/libggi.la -lgg

EXTRA_DIST = speed.sh

MAINTAINERCLEANFILES = Makefile.in

