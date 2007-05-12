AC_INIT([Configuration tools for Frugalware.], 1.0, [frugalware-devel@frugalware.org], frugalwareutils)
AC_PROG_CC
AC_PROG_INSTALL

AC_ARG_ENABLE([doxygen], AS_HELP_STRING([--disable-doxygen], [Enable doxygen support (default: test)]))
if test "$enable_doxygen" != "no"; then
	AC_CHECK_PROG(DOXYGEN, doxygen, doxygen, [])
fi

AC_ARG_ENABLE([po4a], AS_HELP_STRING([--disable-po4a], [Enable po4a support (default: test)]))
if test "$enable_po4a" != "no"; then
	AC_CHECK_PROG(PO4A, po4a, po4a, [])
fi

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Enable debug support (default: disabled)]), debug=yes)
AC_MSG_CHECKING(for debug mode request)
if test x$debug = xyes ; then
	CFLAGS="$CFLAGS -g -Wall -Werror"
	AC_MSG_RESULT(yes)
else
	CFLAGS="$CFLAGS"
	AC_MSG_RESULT(no)
fi

LIB_CFLAGS="$CFLAGS"
case "`$CC -dumpmachine`" in
	i686-*)
		CFLAGS="$CFLAGS -march=i686"
		LIB_CFLAGS="$CFLAGS"
		;;
	x86_64-*)
		CFLAGS="$CFLAGS -march=k8"
		LIB_CFLAGS="$CFLAGS -fPIC"
		;;
esac
AC_SUBST(LIB_CFLAGS)

AC_OUTPUT(config.mak)
