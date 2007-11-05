AC_INIT([Configuration tools for Frugalware.], 1.0, [frugalware-devel@frugalware.org], frugalwareutils)
AC_PROG_CC
AC_PROG_INSTALL

dnl Check for libm
AC_CHECK_LIB([m], [sqrt], [AC_CHECK_HEADER([math.h], [LIBMATH='-lm'])])
if test -n "$LIBMATH"; then
	LDFLAGS="$LDFLAGS $LIBMATH"
else
	AC_MSG_ERROR("math library not found!");
fi

dnl Check for curses
AC_CHECK_LIB([curses], [wmove], [AC_CHECK_HEADER([curses.h], [LIBCURSES='-lcurses'])])
if test -n "$LIBCURSES"; then
	LDFLAGS="$LDFLAGS $LIBCURSES"
else
	AC_MSG_ERROR("curses library not found!");
fi

dnl Check for dialog
AC_CHECK_LIB([dialog], [init_dialog], [AC_CHECK_HEADER([dialog.h], [LIBDIALOG='-ldialog'])])
if test -n "$LIBDIALOG"; then
	LDFLAGS="$LDFLAGS $LIBDIALOG"
else
	AC_MSG_ERROR("dialog library not found!");
fi

dnl Check for glib2
PKG_CHECK_MODULES(GLIB, glib-2.0)

dnl Check for parted
AC_CHECK_LIB([parted], [ped_device_probe_all], [AC_CHECK_HEADER([parted/parted.h], [LIBPARTED='-lparted'])])
if test -n "$LIBPARTED"; then
	LDFLAGS="$LDFLAGS $LIBPARTED"
else
	AC_MSG_ERROR("parted library not found!");
fi

dnl Check for pacman
AC_CHECK_LIB([pacman], [pacman_initialize], [AC_CHECK_HEADER([pacman.h], [LIBPACMAN='-lpacman'])])
if test -n "$LIBPACMAN"; then
	LDFLAGS="$LDFLAGS $LIBPACMAN"
else
	AC_MSG_ERROR("pacman library not found!");
fi

AC_ARG_ENABLE([doxygen], AS_HELP_STRING([--disable-doxygen], [Disable doxygen support (default: test)]))
if test "$enable_doxygen" != "no"; then
	AC_CHECK_PROG(DOXYGEN, doxygen, doxygen, [])
fi

AC_ARG_ENABLE([asciidoc], AS_HELP_STRING([--disable-asciidoc], [Disable asciidoc support (default: test)]))
if test "$enable_asciidoc" != "no"; then
	AC_CHECK_PROG(ASCIIDOC, asciidoc, asciidoc, [])
fi

AC_ARG_ENABLE([po4a], AS_HELP_STRING([--disable-po4a], [Disable po4a support (default: test)]))
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
