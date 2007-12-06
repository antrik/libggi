dnl libgii (un)installed path

AC_DEFUN([GII_INST_PATH],
[

AC_ARG_WITH([gii],
[  --with-gii=DIR          use the LibGII installed DIR],
AM_CPPFLAGS="$AM_CPPFLAGS -I$withval/include"
  AM_LDFLAGS="$AM_LDFLAGS -L$withval/lib"
  CPPFLAGS="$CPPFLAGS -I$withval/include"
  LDFLAGS="$LDFLAGS -L$withval/lib"
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-gii=$withval",
AM_CPPFLAGS="$AM_CPPFLAGS -I$prefix/include"
  AM_LDFLAGS="$AM_LDFLAGS -L$prefix/lib"
  CPPFLAGS="$CPPFLAGS -I$prefix/include"
  LDFLAGS="$LDFLAGS -L$prefix/lib"
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-gii=$prefix")

])

AC_DEFUN([GII_UNINST_PATH],
[

# This is for building against an uninstalled libgii for
# both $(top_builddir) == $(top_srcdir)
# and $(top_builddir) != $(top_srcdir)
AC_ARG_WITH([uninst-gii],
[  --with-uninst-gii=DIR   use uninstalled copy of LibGII found in DIR],
[[if test -d "$withval" ; then
     gii_top_builddir="`cd "$withval" ; pwd`"
     gii_top_srcdir="$gii_top_builddir/`cd "$withval" ; sed -n -e 's/^top_srcdir[ 	]*=[ 	]*//p' Makefile`"
     gii_top_srcdir="`cd $gii_top_srcdir ; pwd`"
     DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-uninst-gii=$gii_top_builddir"
   fi
   if test -d "$gii_top_builddir" -a -d "$gii_top_srcdir" ; then
     if test "$gii_top_builddir" = "$gii_top_srcdir" ; then
       AM_CPPFLAGS="-I$gii_top_builddir/include $AM_CPPFLAGS"
       CPPFLAGS="-I$gii_top_builddir/include $CPPFLAGS"
     else
       AM_CPPFLAGS="-I$gii_top_srcdir/include -I$gii_top_builddir/include $AM_CPPFLAGS"
       CPPFLAGS="-I$gii_top_srcdir/include -I$gii_top_builddir/include $CPPFLAGS"
     fi
     AM_LDFLAGS="-L$gii_top_builddir/gii $AM_LDFLAGS"
     LDFLAGS="-L$gii_top_builddir/gii $LDFLAGS"
   fi
]])

])
