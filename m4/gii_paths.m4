dnl libgii (un)installed path

AC_DEFUN([GII_INST_PATH],
[

AC_ARG_WITH([gii],
[  --with-gii=DIR          use the LibGII installed DIR],
CFLAGS="$CFLAGS -I$withval/include"
  CPPFLAGS="$CPPFLAGS -I$withval/include"
  LDFLAGS="$LDFLAGS -L$withval/lib",
CFLAGS="$CFLAGS -I$prefix/include"
  CPPFLAGS="$CPPFLAGS -I$prefix/include"
  LDFLAGS="$LDFLAGS -L$prefix/lib")

])

AC_DEFUN([GII_UNINST_PATH],
[

# This is for building against an uninstalled libgii for
# both $(top_builddir) == $(top_srcdir)
# and $(top_builddir) != $(top_srcdir)
# I'd like the sed expression to be more robust, but
# character set matching '[]' is stripped by m4
AC_ARG_WITH([uninst-gii],
[  --with-uninst-gii=DIR   use uninstalled copy of LibGII found in DIR],
[[if test -d "$withval" ; then
     gii_top_builddir="`cd "$withval" ; pwd`"
     gii_top_srcdir="$gii_top_builddir/`cd "$withval" ; sed -n -e 's/^top_srcdir[       ]*=[    ]*//p' Makefile`"
     gii_top_srcdir="`cd $gii_top_srcdir ; pwd`"
   fi
   if test -d "$gii_top_builddir" -a -d "$gii_top_srcdir" ; then
     if test "$gii_top_builddir" = "$gii_top_srcdir" ; then
       CFLAGS="-I$gii_top_builddir/include $CFLAGS"
       CPPFLAGS="-I$gii_top_builddir/include $CPPFLAGS"
     else
       CFLAGS="-I$gii_top_srcdir/include -I$gii_top_builddir/include $CFLAGS"
       CPPFLAGS="-I$gii_top_srcdir/include -I$gii_top_builddir/include $CPPFLAGS"
     fi
     LDFLAGS="-L$gii_top_builddir/gg -L$gii_top_builddir/gii $LDFLAGS"
   fi
]])

])
