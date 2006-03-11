# GGOBI_CONFIG_PLUGIN()
# -------------------------------------------------------------
#
# Helper macro for configuring GGobi plugins

AC_DEFUN([GGOBI_CONFIG_PLUGIN],
[AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
AC_C_CONST
AC_HEADER_STDBOOL
AC_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL

AC_ARG_ENABLE(debug, [--enable-debug          Compile with debugging symbols])
AC_ARG_ENABLE(local, [--enable-local          Configure for use directly from this directory tree.])

if test -n "$enable_local" && test "$enable_local" != "no"; then
  PKG_CONFIG_PATH="../..:$PKG_CONFIG_PATH"
  export PKG_CONFIG_PATH
fi

PKG_CHECK_MODULES(GGOBI, [ggobi],,[AC_MSG_ERROR([You must have GGobi to use this plugin!])])
AC_SUBST(GGOBI_FLAGS)
AC_SUBST(GGOBI_LIBS)

if test -n "$enable_debug" && ! test "$enable_debug" = "no" ;  then
 SRC_DEBUG="-g -Wall"
fi
AC_SUBST(SRC_DEBUG)
])

AC_DEFUN([GGOBI_TEST],
[echo "HELLO WORLD"])
