# This is the canonical configure.ac for plugins.

# First, check our autoconf version
AC_PREREQ(2.59)

# Init autoconf
AC_INIT([GraphLayout], 1.0, dfs@research.att.com)

# Point autoconf to a particular source file.
AC_CONFIG_SRCDIR([glayout.c])

# Put all the extra scripts and stuff in the current directory
AC_CONFIG_AUX_DIR([.])

# Generate config.h
AC_CONFIG_HEADER([config.h])

# Initialize automake
AM_INIT_AUTOMAKE

# Use the custom GGobi macro for configuring the plugin. This provides 
# GGOBI_CFLAGS, GGOBI_LIBS, and SRC_DEBUG variables.
GGOBI_CONFIG_PLUGIN([../..])
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(LIBGVC, libgvc,
	AC_DEFINE_UNQUOTED([HAVE_LIBGVC], [1], [whether the system has support for libgvc - graphviz]),
	[HAVE_LIBGVC=0])
AC_SUBST(LIBGVC_CFLAGS)
AC_SUBST(LIBGVC_LIBS)

# Check for 'bool' type to avoid graphviz logic.h issue
AC_MSG_CHECKING([for bool])
AC_TRY_COMPILE([
    #ifdef HAVE_STDBOOL_H>
    #include <stdbool.h>
    #endif
    ],[
    bool foo = true, bar = false;
    ],[
AC_MSG_RESULT(yes)
AC_DEFINE(HAVE_BOOL, 1,[Define to 1 if compiler supports bool])
    ],[
AC_MSG_RESULT(no)])

# Finally, output the configuration
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

