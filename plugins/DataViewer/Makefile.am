# This is the basic structure of every plugin Makefile.am

# This line is specific to plugins that are part of the GGobi distribution
# It tells automake to import the helper macros in /m4 into this
# plugin's aclocal.m4.
# These macros should be installed into a system directory (/usr/share/aclocal)
# along with GGobi, so an independent plugin shouldn't have to do this.
ACLOCAL_AMFLAGS = -I ../../m4

# Every plugin provides a single library to GGobi, define it here.
# Its name should start with 'lib' and end with '.la'.
lib_LTLIBRARIES        = libdataviewer.la

# Most plugins will only require GGobi and its dependencies (gtk, libxml2)
libdataviewer_la_LDFLAGS  = @GGOBI_LIBS@
libdataviewer_la_CFLAGS   = @GGOBI_CFLAGS@ @SRC_DEBUG@

# List the source files of the plugin here.
libdataviewer_la_SOURCES  = dataviewer.c
