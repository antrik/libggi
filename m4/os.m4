dnl Add your OS here, if not listed

AC_DEFUN(GGI_CHECKOS,
[

case "${host_os}" in
  cygwin* | mingw* | pw32*)
	os="os_win32"
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
  *)
	os="os_default"
        ;;
esac

AM_CONDITIONAL(OS_WIN32, test $os = "os_win32")
AM_CONDITIONAL(OS_DARWIN, test $os = "os_darwin")
AM_CONDITIONAL(OS_FREEBSD, test $os = "os_freebsd")
AM_CONDITIONAL(OS_LINUX, test $os = "os_linux")
AM_CONDITIONAL(OS_NETBSD, test $os = "os_netbsd")
AM_CONDITIONAL(OS_OPENBSD, test $os = "os_openbsd")
AM_CONDITIONAL(OS_SOLARIS, test $os = "os_solaris")
AM_CONDITIONAL(OS_DEFAULT, test $os = "os_default")

])
