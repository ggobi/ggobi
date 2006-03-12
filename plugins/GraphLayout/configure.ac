# This is the canonical configure.ac for plugins.

# First, check our autoconf version
AC_PREREQ(2.59)

# Init autoconf
AC_INIT([graphlayout], 1.0, dfs@research.att.com)

# Point autoconf to a particular source file.
AC_CONFIG_SRCDIR([glayout.c])

# Put all the extra scripts and stuff in the current directory
AC_CONFIG_AUX_DIR([.])

# Use the custom GGobi macro for configuring the plugin. This provides 
# GGOBI_CFLAGS, GGOBI_LIBS, and SRC_DEBUG variables.
GGOBI_CONFIG_PLUGIN([../..])
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(LIBGVC, [libgvc >= 2.3],
        [HAVE_LIBGVC="1"])
AC_SUBST(LIBGVC_CFLAGS)
AC_SUBST(LIBGVC_LIBS)

# Finally, output the configuration
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
