INCLUDES = -I${top_srcdir}/include
LDADD = ${top_builddir}/ggi/libggi.la -lgii -lgg

EXTRA_PROGRAMS = showaccel2 speed
noinst_PROGRAMS = checkmode showaccel findleaks \
	consistency structs cbconsist pointer palette \
	snprintmode $(OPTCHECKS)

# -lgii is not needed
checkmode_LDADD =  ${top_builddir}/ggi/libggi.la -lgg

EXTRA_DIST = speed.sh

MAINTAINERCLEANFILES = Makefile.in
