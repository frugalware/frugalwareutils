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

AC_OUTPUT(config.mak)
