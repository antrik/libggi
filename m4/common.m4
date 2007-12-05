dnl ------ compiler.m4 ------
dnl Check CC specific characteristics

dnl $1 is the $CC option to check for.
dnl $2 is a boolean value (1 for true or 0 for false),
dnl    whether the option should be activated in
dnl    CFLAGS if available
dnl    Defaults to true, if not specified

AC_DEFUN([GGI_CC_CHECK4_OPTION],
[

AS_VAR_PUSHDEF([ggi_Option], [AS_TR_SH(m4_tolower([ggi_cv_cc_has_$1]))])dnl
AC_CACHE_CHECK([if $CC has -$1 option], [ggi_Option],
[
save_CFLAGS="$CFLAGS"

CFLAGS="$CFLAGS -$1"
AC_COMPILE_IFELSE([
	AC_LANG_PROGRAM([[#include <stdio.h>]],
			[[printf("Hello World\n");]])
	], [cc_tmp=yes],
	[cc_tmp=no])
AS_VAR_SET([ggi_Option], [$cc_tmp])
CFLAGS="$save_CFLAGS"
])

activate=1
if test -n "$2"; then
	activate=$2
fi

if test $activate -ne 0 -a AS_VAR_GET([ggi_Option]) = yes; then
	AM_CFLAGS="$AM_CFLAGS -$1"
fi

AS_VAR_SET(AS_TR_SH(m4_tolower([cc_has_$1])), AS_VAR_GET([ggi_Option]))

AS_VAR_POPDEF([ggi_Option])dnl

])



dnl Check if -Werror-implicit-function-declaration kills the build.

dnl AIX 4.2 is missing a prototype for (at least) snprintf.
dnl Cygwin 1.5.11 is missing a prototype for (at least) siginterrupt.
dnl HP/UX 11.11 is missing a prototype for (at least) strtoull.
dnl True64 (version?) is missing a prototype for (at least) strtoull.

AC_DEFUN([GGI_CHECK_IMPLICIT_DECLARATIONS],
[

dnl First check, if we _have_ -Werror-implicit-function-declaration
GGI_CC_CHECK4_OPTION([Werror-implicit-function-declaration], 0)

if test "$cc_has_werror_implicit_function_declaration" = "yes"; then
	ggi_test_funcs="strncasecmp snprintf siginterrupt strtoull"

	dnl Now check for some headers ...
	AC_CHECK_HEADERS([string.h strings.h signal.h])

	dnl ... for the declarations
	for func in $ggi_test_funcs; do
		AC_CHECK_DECLS($func,[],[],
			[$ac_includes_default
			 #ifdef HAVE_SIGNAL_H
			 #include <signal.h>
			 #endif])
	done

	dnl ... and for the symbols.
	AC_CHECK_FUNCS($ggi_test_funcs)

	AC_MSG_CHECKING([if the system is missing declarations])

	for func in $ggi_test_funcs; do
		decl_var=AS_TR_SH([ac_cv_have_decl_$func])
		func_var=AS_TR_SH([ac_cv_func_$func])

		if eval "test \"x\${$decl_var}\" = \"xno\" -a \
			      \"x\${$func_var}\" = \"xyes\""; then
		  cc_has_werror_implicit_function_declaration="no"
		fi
	done

	dnl When we can use -Werror-implicit-function-declaration, then
	dnl there are no declarations missing. Thus the result output
	dnl must be inverted!
	if test $cc_has_werror_implicit_function_declaration = "yes"; then
		AC_MSG_RESULT([no])
	else
		AC_MSG_RESULT([yes])
	fi
fi

])

AC_DEFUN([GGI_CC_SET_DEFAULT_OPTIONS],
[
   GGI_CC_CHECK4_OPTION([Wall])
   GGI_CC_CHECK4_OPTION([pedantic])
   GGI_CHECK_IMPLICIT_DECLARATIONS
   GGI_CC_CHECK4_OPTION([std=gnu99])
   GGI_CC_CHECK4_OPTION([Wpointer-arith])
   GGI_CC_CHECK4_OPTION([Wsign-compare])
   GGI_CC_CHECK4_OPTION([Wstrict-prototypes])
   GGI_CC_CHECK4_OPTION([Wswitch])
   GGI_CC_CHECK4_OPTION([Wmissing-prototypes])
   GGI_CC_CHECK4_OPTION([Wreturn-type])
   GGI_CC_CHECK4_OPTION([Wshadow])
   GGI_CC_CHECK4_OPTION([Wnested-externs])
   GGI_CC_CHECK4_OPTION([Wredundant-decls])
   GGI_CC_CHECK4_OPTION([Wuninitialized])
   GGI_CC_CHECK4_OPTION([Wcast-qual])
   GGI_CC_CHECK4_OPTION([Wwrite-strings])
   GGI_CC_CHECK4_OPTION([Wmissing-include-dirs])
   GGI_CC_CHECK4_OPTION([Wdeclaration-after-statement])

   if test "x$cc_has_werror_implicit_function_declaration" = "xyes"; then
      AM_CFLAGS="$AM_CFLAGS -Werror-implicit-function-declaration"
   fi
])

dnl ----- dllext.m4 -----
dnl Suffix detection for runtime loadable libraries.

AC_DEFUN([GGI_DLLEXT],
[

AC_MSG_CHECKING([for shared library extension])
case "${host}" in
  *-*-mingw* | *-*-cygwin*)
	DLLEXT="dll"
        ;;
  *)
	DLLEXT="so"
        ;;
esac
AC_MSG_RESULT([$DLLEXT])

AC_SUBST(DLLEXT)

])




dnl ------ extrapaths.m4 ------
dnl Let user add extra includes and libs

AC_DEFUN([GGI_EXTRA_PATHS],
[

AC_ARG_WITH([extra-includes],
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
  AM_CPPFLAGS="$AM_CPPFLAGS $extra_includes"
  CPPFLAGS="$CPPFLAGS $extra_includes"
fi

AC_ARG_WITH([extra-libs],
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
   AM_LDFLAGS="$AM_LDFLAGS $extra_libraries"
   LDFLAGS="$LDFLAGS $extra_libraries"
fi

AC_SUBST(extra_includes)
AC_SUBST(extra_libraries)

])



dnl ------ os.m4 ------
dnl Add your OS here, if not listed

AC_DEFUN([GGI_CHECKOS],
[

case "${host_os}" in
  cygwin*)
	os="os_win32_cygwin"
	;;
  mingw*)
	os="os_win32_mingw"
	;;
  pw32*)
	os="os_win32_pw32"
	;;
  darwin* | rhapsody*)
	os="os_darwin"
        ;;
  freebsd*)
	os="os_freebsd"
	;;
  linux*)
	os="os_linux"
	;;
  netbsd*)
	os="os_netbsd"
	;;
  openbsd*)
	os="os_openbsd"
	;;
  solaris*)
	os="os_solaris"
	;;
  aix*)
	os="os_aix"
	;;
  *)
	os="os_default"
        ;;
esac

AM_CONDITIONAL(OS_WIN32, [test $os = "os_win32_cygwin" -o $os = "os_win32_mingw" -o $os = "os_win32_pw32"])
AM_CONDITIONAL(OS_WIN32_CYGWIN, test $os = "os_win32_cygwin")
AM_CONDITIONAL(OS_WIN32_MINGW, test $os = "os_win32_mingw")
AM_CONDITIONAL(OS_WIN32_PW32, test $os = "os_win32_pw32")

AM_CONDITIONAL(OS_DARWIN, test $os = "os_darwin")
AM_CONDITIONAL(OS_FREEBSD, test $os = "os_freebsd")
AM_CONDITIONAL(OS_LINUX, test $os = "os_linux")
AM_CONDITIONAL(OS_NETBSD, test $os = "os_netbsd")
AM_CONDITIONAL(OS_OPENBSD, test $os = "os_openbsd")
AM_CONDITIONAL(OS_SOLARIS, test $os = "os_solaris")
AM_CONDITIONAL(OS_AIX, test $os = "os_aix")
AM_CONDITIONAL(OS_DEFAULT, test $os = "os_default")

])




dnl ------ rellibdir.m4 -----
dnl Find a relative path that takes us from static_sysconfdir/ggi_subdir
dnl to static_libdir/ggi_subdir.

dnl This macro is easily fooled with //, /./ and /../ combinations.
dnl Also, I'm very far from a script guru. This macro can probably
dnl be written in a couple of lines if you know the right script
dnl knobs.

AC_DEFUN([GGI_SYSCONF_TO_LIB],
[
rel_ggisysconfdir="$static_sysconfdir/$ggi_subdir"
rel_ggilibdir="$static_libdir/$ggi_subdir"
again="yes"
until test "$again" = "no"; do
	again=""
	newcdir=""
	newldir=""
	as_save_IFS=$IFS
	IFS='/'
	for cdir in $rel_ggisysconfdir; do
		IFS=$as_save_IFS
		if test -n "$again"; then
			if test -n "$newcdir"; then
				newcdir="$newcdir/$cdir"
			else
				newcdir="$cdir"
			fi
			continue
		fi
		as_save_IFS=$IFS
		IFS='/'
		for ldir in $rel_ggilibdir; do
			IFS=$as_save_IFS
			if test -z "$again"; then
				if test "$ldir" = "$cdir"; then
					again="yes"
					newldir=""
					newcdir=""
				else
					again="no"
					newldir="$ldir"
					newcdir="$cdir"
				fi
				continue
			fi
			if test -n "$newldir"; then
				newldir="$newldir/$ldir"
			else
				newldir="$ldir"
			fi
		done
	done
	rel_ggisysconfdir="$newcdir"
	rel_ggilibdir="$newldir"
done
as_save_IFS=$IFS
IFS='/'
for cdir in $rel_ggisysconfdir; do
	rel_ggilibdir="../$rel_ggilibdir"
done
IFS=$as_save_IFS

ggi_sysconfdir_to_libdir="$rel_ggilibdir"
])


dnl ----- strings.m4 -----
dnl Check for safe and secure string functions

AC_DEFUN([GGI_CHECK_STRING_FUNCS],
[

dnl strings.h exists on SYSV systems
dnl and contains strcasecmp() and strncasecmp()
AC_CHECK_HEADERS(string.h strings.h)

AC_CHECK_FUNCS([strcasecmp strncasecmp \
	strncat strncpy snprintf vsnprintf \
	strlcpy strlcat asprintf \
	strtol strtoul strtoll strtoull])

])



dnl ----- checklib.m4 -----
dnl Check for libs using libtool

AC_DEFUN([GGI_CHECK_LIB],
[
   save_CC="$CC"
   CC="$SHELL ./libtool --mode=link $CC"
   AC_CHECK_LIB($1, $2, [
     CC="$save_CC"
     $3], [
     CC="$save_CC"
     $4])
])

dnl ----- checkint.m4 -----
dnl Add fallback if C99 inttypes are not there...
AC_DEFUN([GGI_NEED_INTTYPES],
[
if test "$ac_cv_header_inttypes_h" != "yes"; then
  AC_DEFINE(GG_NEED_INTTYPES, 1,
	    [Tell libgg to define integer types when building])
fi
])


dnl ------ checktarget.m4 ----
dnl Check whether to build a target
dnl Requires 7 arguments
dnl $1: target name
dnl $2: build_ variable
dnl $3: *SUBDIRS
dnl $4: *MODULES
dnl $5: directory
dnl $6: libname (.la)
dnl $7: BUILTIN_* ac_define
dnl $8: Optional: quoted action on failure
dnl $9: Optional: quoted action on success
dnl Hint: It is not possible to nest GGI_CHECK_TARGET
dnl     because AM_CONDITIONAL may not be within an if

AC_DEFUN([GGI_CHECK_TARGET],
[
  AC_MSG_CHECKING([if we should build ]$1)
  if test "x$$2" = "xno"; then
    $8
    AC_MSG_RESULT([no])
  else
    $3="$$3 $5"
    if test "$enable_shared" = "yes"; then
      $4="$$4 $6"
    fi
    if test "$enable_static" = "yes"; then
      AC_DEFINE($7, 1, [Support for builtin ]$1)
    fi
    $2="yes"
    $9
    AC_MSG_RESULT([yes])
  fi
  AM_CONDITIONAL($7,
		test "$enable_static" = "yes" -a \
			"x$$2" != "xno")
])



dnl User variables
dnl We want to use AM_CFLAGS et al during configure, but
dnl autoconf is not fully automake aware. So, we have to
dnl reuse CFLAGS et al during configure to mimic what
dnl happens during build.
AC_DEFUN([GGI_SAVE_USER_VARS],
[
	ggi_save_user_CFLAGS="$CFLAGS"
	ggi_save_user_CPPFLAGS="$CPPFLAGS"
	ggi_save_user_LDFLAGS="$LDFLAGS"
])

dnl User variables
AC_DEFUN([GGI_RESTORE_USER_VARS],
[
	CFLAGS="$ggi_save_user_CFLAGS"
	CPPFLAGS="$ggi_save_user_CPPFLAGS"
	LDFLAGS="$ggi_save_user_LDFLAGS"
])


# GGI_REPEAT(COUNT, VALUE)
# --------------------------------------------------------
# Repeat VALUE COUNT times, with a comma between each invocation
AC_DEFUN([GGI_REPEAT],
[m4_for([index],[0],[$1],[+1],[m4_case(m4_eval(index),[$1],[],m4_eval([$1 - 1]),[$2],[$2,])])])


# GGI_SEARCH_LIBS(FUNCTION, ARG-COUNT, SEARCH-LIBS,
#                 [OTHER-LIBRARIES], [PROLOGUE])
# --------------------------------------------------------
# Search for a library defining FUNC, if it's not already available.
AC_DEFUN([GGI_SEARCH_LIBS],
[AS_VAR_PUSHDEF([ggi_Search], [ggi_cv_search_$1])dnl
AC_CACHE_CHECK([for library containing $1], [ggi_Search],
[ggi_func_search_save_LIBS=$LIBS
AC_LANG_CONFTEST([AC_LANG_PROGRAM([$5
#ifdef __cplusplus
extern "C"
#endif
char $1( GGI_REPEAT([$2], [int]) );],
[return $1( GGI_REPEAT([$2], [0]) );])])
for ggi_lib in '' $3; do
	if test -z "$ggi_lib"; then
		ggi_res="none required"
	else
		ggi_res=-l$ggi_lib
		LIBS="-l$ggi_lib $4 $ggi_func_search_save_LIBS"
	fi
	AC_LINK_IFELSE([], [AS_VAR_SET([ggi_Search], [$ggi_res])])
	AS_VAR_SET_IF([ggi_Search], [break])dnl
done
AS_VAR_SET_IF([ggi_Search], , [AS_VAR_SET([ggi_Search], [no])])dnl
rm conftest.$ac_ext
LIBS=$ggi_func_search_save_LIBS])
AS_VAR_POPDEF([ggi_Search])dnl
])


dnl ------ checktarget.m4 ---- 
dnl Check how to handle getaddrinfo

AC_DEFUN([GGI_FUNC_GETADDRINFO],
[

GGI_SEARCH_LIBS(getaddrinfo, 4, [socket ws2_32], [], [
#ifdef _WIN32
__stdcall
#endif
])

if test "$ggi_cv_search_getaddrinfo" != "no"; then
	AC_DEFINE(HAVE_GETADDRINFO, 1,
		[Define to 1 if you have getaddrinfo])
	if test "$ggi_cv_search_getaddrinfo" != "none required"; then
		GAI_LIB="$ggi_cv_search_getaddrinfo"
	fi
fi

])
