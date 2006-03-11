ACLOCAL_AMFLAGS = -I ../../m4

lib_LTLIBRARIES        = graphlayout.la
graphlayout_la_LDFLAGS  = @GGOBI_LIBS@ $(LIBGVC_LIBS) -module
graphlayout_la_CFLAGS   = @GGOBI_CFLAGS@ @SRC_DEBUG@ $(LIBGVC_CFLAGS) -DHAVE_LIBGVC -Wno-unknown-pragmas
graphlayout_la_SOURCES  = cmds.c glayout.c graphviz.c init.c radial.c
