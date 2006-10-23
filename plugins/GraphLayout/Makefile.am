ACLOCAL_AMFLAGS = -I ../../m4

include ../Makefile.plugin

plugin_la_LDFLAGS += $(LIBGVC_LIBS)
plugin_la_CFLAGS += $(LIBGVC_CFLAGS) -Wno-unknown-pragmas

plugin_la_SOURCES = glayout.c graphviz.c init.c radial.c
noinst_HEADERS  = glayout.h defines.h

EXTRA_DIST += Docs/GraphLayout.tex Docs/references.bib
