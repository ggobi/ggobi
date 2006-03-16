# This line is specific to plugins that are part of the GGobi distribution
# It tells automake to import the helper macros in /m4 into this
# plugin's aclocal.m4.
# These macros should be installed into a system directory (/usr/share/aclocal)
# along with GGobi, so an independent plugin shouldn't have to do this.
ACLOCAL_AMFLAGS = -I ../../m4

include ../Makefile.plugin

# List the source files of the plugin here.
plugin_la_SOURCES  = dataviewer.c
