dnl Let user add extra includes and libs

AC_DEFUN(GGI_DLLEXT,
[

AC_MSG_CHECKING(for shared library extension)
case "${host}" in
  *-*-mingw* | *-*-cygwin*)
	DLLEXT="dll"
        ;;
  *-*-darwin* | *-*-rhapsody*)
	DLLEXT="dylib"
	;;
  *)
	DLLEXT="so"
        ;;
esac
AC_MSG_RESULT($DLLEXT)

AC_SUBST(DLLEXT)

])
