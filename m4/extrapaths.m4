dnl Let user add extra includes and libs

AC_DEFUN(GGI_EXTRA_PATHS,
[

AC_ARG_WITH(extra-includes,
[  --with-extra-includes=DIR
                          add extra include paths (separator ':')],
  use_extra_includes="$withval",
  use_extra_includes=NO
)

if test -n "$use_extra_includes" && \
        test "$use_extra_includes" != "NO"; then
  ac_save_ifs=$IFS
  IFS=':'
  for dir in ${use_extra_includes} ; do
    if test -d "$dir" ; then
      extra_includes="$extra_includes -I$dir"
    else
      test -f "$dir" && \
        extra_includes="$extra_includes -include $dir"
    fi
  done
  IFS=$ac_save_ifs
  CPPFLAGS="$CPPFLAGS $extra_includes"
fi

AC_ARG_WITH(extra-libs,
[  --with-extra-libs=DIR   add extra library paths (separator ':')],
  use_extra_libs="$withval",
  use_extra_libs=NO
)
if test -n "$use_extra_libs" && \
        test "$use_extra_libs" != "NO"; then
   ac_save_ifs=$IFS
   IFS=':'
   for dir in $use_extra_libs; do
     if test -d "$dir" ; then
       extra_libraries="$extra_libraries -L$dir"
     else
       extra_libraries="$extra_libraries -l$dir"
     fi
   done
   IFS=$ac_save_ifs
fi

AC_SUBST(extra_includes)
AC_SUBST(extra_libraries)

])
