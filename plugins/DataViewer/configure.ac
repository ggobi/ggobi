# This is the canonical configure.ac for plugins.

# First, check our autoconf version
AC_PREREQ(2.59)

# Init autoconf
AC_INIT([dataviewer], 0.1, lawremi@iastate.edu)

# Point autoconf to a particular source file.
AC_CONFIG_SRCDIR([dataviewer.c])

# Put all the extra scripts and stuff in the current directory
AC_CONFIG_AUX_DIR([.])

# Use the custom GGobi macro for configuring the plugin. This provides 
# GGOBI_CFLAGS, GGOBI_LIBS, and SRC_DEBUG variables. It's smart enough to 
# build against a local GGobi if necessary.
GGOBI_CONFIG_PLUGIN
# Put this here to make autoreconf happy
AC_PROG_LIBTOOL

# Finally, output the configuration
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
