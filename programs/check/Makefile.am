INCLUDES = -I${top_srcdir}/include ${extra_includes}
LDADD = ${top_builddir}/ggi/libggi.la

EXTRA_PROGRAMS = showaccel2 speed
noinst_PROGRAMS = checkmode showaccel findleaks \
	consistency structs cbconsist pointer \
	$(OPTCHECKS)

# -lgg is needed for ggUSleep() and ggCurTime()
showaccel_LDADD =  ${top_builddir}/ggi/libggi.la -lgg

EXTRA_DIST = speed.sh
