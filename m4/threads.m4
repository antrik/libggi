dnl Requires GGI_CHECKOS have been run before
dnl Return: $mutextype and $THREADLIB

AC_DEFUN([GGI_THREADLIBS],
[

use_threads="auto"
threadtype=""
THREADLIBS=""


dnl Check for pthread library

if test "$use_threads" != "no"; then

AC_CHECK_HEADERS([pthread.h])
have_pthread=no
if test "$ac_cv_header_pthread_h" = "yes"; then
	AC_MSG_CHECKING(for pthread library)
	TMP_SAVE_LIBS=$LIBS
	LIBS="$LIBS -lpthread"

	AC_TRY_RUN([
		#define __C_ASM_H /* fix for retarded Digital Unix headers */
		#include <pthread.h>
		pthread_mutex_t mtex;
		int main(void)
		{
			if (pthread_mutex_init(&mtex, NULL) == 0) return 0;
			return 1;
		}
	],[
		AC_MSG_RESULT(yes)
		have_pthread=yes
		THREADLIBS="-lpthread"
	],[
		AC_MSG_RESULT(no)
		case "$host_os" in
		openbsd3.0 | freebsd5.1 | freebsd4.*)
			_pthread="-pthread"
			;;
		*)
			_pthread="-lc_r"
			;;
		esac

		AC_MSG_CHECKING(for pthread_mutex_init in $_pthread)
		LIBS="$TMP_SAVE_LIBS $_pthread"
		AC_TRY_RUN([
			#include <pthread.h>
			pthread_mutex_t mtex;
			int main(void)
			{
				if (pthread_mutex_init(&mtex, NULL) == 0) return 0;
				return 1;
			}
		],[
			AC_MSG_RESULT(yes)
			have_pthread=yes
			THREADLIBS="$_pthread"
		],[
			AC_MSG_RESULT(no)
		])
	])
	LIBS=$TMP_SAVE_LIBS
fi

dnl Use pthread library if present
if test "$have_pthread" = "yes"; then
	use_threads="yes"
	threadtype="pthread"
fi

fi


dnl Check for OS specific thread support

if test "$use_threads" != "no"; then

dnl Win32 (after pthread since pthread is prefered on cygwin)
AC_CHECK_HEADERS([windows.h])
AC_MSG_CHECKING(for win32 semaphores)
if test "x$ac_cv_header_windows_h" = "xyes"; then
	AC_MSG_RESULT(yes)
	use_threads="yes"
	threadtype="win32"
	THREADLIBS=""
else
	AC_MSG_RESULT(no)
fi

fi


dnl Fall back to not use threads, if nothing else was found

if test "$use_threads" != "yes"; then
	use_threads="no"
fi

AC_SUBST(THREADLIBS)

])
