# GGOBI_CONFIG_PLUGIN(GGOBI_ROOT)
# -------------------------------------------------------------
#
# Helper macro for configuring GGobi plugins

AC_DEFUN([GGOBI_CONFIG_PLUGIN],
[
AC_C_CONST
AC_HEADER_STDBOOL
AC_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL

AC_ARG_ENABLE(debug, [--enable-debug          Compile with debugging symbols])
AC_ARG_WITH(ggobi, [--with-ggobi          Compile plugin against the GGobi in a specified directory])

if test -n "$1"; then
  GGOBI_PKG_CONFIG_PATH="$1"
fi

if test -n "$with_ggobi" && ! test "$with_ggobi" = "no" ;  then
  GGOBI_PKG_CONFIG_PATH="$with_ggobi"
fi

if test -n "$PKG_CONFIG_PATH"; then
  PKG_CONFIG_PATH="$GGOBI_PKG_CONFIG_PATH:$PKG_CONFIG_PATH"
else
  PKG_CONFIG_PATH="$GGOBI_PKG_CONFIG_PATH"
fi

echo "pkgconfig path: ${PKG_CONFIG_PATH}"
export PKG_CONFIG_PATH

PKG_CHECK_MODULES(GGOBI, [ggobi])
AC_SUBST(GGOBI_FLAGS)
AC_SUBST(GGOBI_LIBS)

if test -n "$enable_debug" && ! test "$enable_debug" = "no" ;  then
 SRC_DEBUG="-g -Wall"
fi
AC_SUBST(SRC_DEBUG)

AC_CANONICAL_HOST
AC_MSG_CHECKING([for native Win32])
case "$host" in
  *-*-mingw*)
    os_win32=yes
    ;;
  *)
    os_win32=no
    ;;
esac
echo "${os_win32}"
AM_CONDITIONAL(OS_WIN32, test "$os_win32" = "yes")
])

AC_DEFUN([GGOBI_TEST],
[echo "HELLO WORLD"])
