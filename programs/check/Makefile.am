INCLUDES = -I${top_srcdir}/include ${extra_includes}
LDADD = ${top_builddir}/ggi/libggi.la

noinst_PROGRAMS = checkmode showaccel showaccel2 findleaks speed \
	consistency structs cbconsist

# -lgg is needed for ggUSleep() and ggCurTime()
showaccel_LDADD =  ${top_builddir}/ggi/libggi.la -lgg

EXTRA_DIST = speed.sh
