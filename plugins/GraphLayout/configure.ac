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

AC_MSG_CHECKING([whether Graphviz API is based on cgraph])
CFLAGS=$LIBGVC_CFLAGS
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
   #include <gvc.h>
   int main() {
     Agraph_t *graph;
     graph = agopen ("BrowserCanvasLayout", Agdirected, NULL);
     return 0;
   }
])], graphviz_new_api=yes, graphviz_new_api=no)
AC_MSG_RESULT($graphviz_new_api)
if test "$graphviz_new_api" = "yes"; then
  AC_DEFINE(HAVE_CGRAPH,[1],[define for cgraph-based graphviz])
fi

# Finally, output the configuration
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

