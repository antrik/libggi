dnl libgg (un)installed path

AC_DEFUN([GG_INST_PATH],
[

AC_ARG_WITH([gg],
[  --with-gg=DIR          use the LibGG installed DIR],
AM_CPPFLAGS="$AM_CPPFLAGS -I$withval/include"
  AM_LDFLAGS="$AM_LDFLAGS -L$withval/lib"
  CPPFLAGS="$CPPFLAGS -I$withval/include"
  LDFLAGS="$LDFLAGS -L$withval/lib"
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-gg=$withval",
AM_CPPFLAGS="$AM_CPPFLAGS -I$prefix/include"
  AM_LDFLAGS="$AM_LDFLAGS -L$prefix/lib"
  CPPFLAGS="$CPPFLAGS -I$prefix/include"
  LDFLAGS="$LDFLAGS -L$prefix/lib"
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-gg=$prefix")

])

AC_DEFUN([GG_UNINST_PATH],
[

# This is for building against an uninstalled libgg for
# both $(top_builddir) == $(top_srcdir)
# and $(top_builddir) != $(top_srcdir)
# I'd like the sed expression to be more robust, but
# character set matching '[]' is stripped by m4
AC_ARG_WITH([uninst-gg],
[  --with-uninst-gg=DIR   use uninstalled copy of LibGG found in DIR],
[[if test -d "$withval" ; then
     gg_top_builddir="`cd "$withval" ; pwd`"
     gg_top_srcdir="$gg_top_builddir/`cd "$withval" ; sed -n -e 's/^top_srcdir[       ]*=[    ]*//p' Makefile`"
     gg_top_srcdir="`cd $gg_top_srcdir ; pwd`"
     DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-uninst-gg=$gg_top_builddir"
   fi
   if test -d "$gg_top_builddir" -a -d "$gg_top_srcdir" ; then
     if test "$gg_top_builddir" = "$gg_top_srcdir" ; then
       AM_CPPFLAGS="-I$gg_top_builddir/include $AM_CPPFLAGS"
       CPPFLAGS="-I$gg_top_builddir/include $CPPFLAGS"
     else
       AM_CPPFLAGS="-I$gg_top_srcdir/include -I$gg_top_builddir/include $AM_CPPFLAGS"
       CPPFLAGS="-I$gg_top_srcdir/include -I$gg_top_builddir/include $CPPFLAGS"
     fi
     AM_LDFLAGS="-L$gg_top_builddir/gg $AM_LDFLAGS"
     LDFLAGS="-L$gg_top_builddir/gg $LDFLAGS"
   fi
]])

])
