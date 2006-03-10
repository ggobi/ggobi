ACLOCAL_AMFLAGS = -I ../../m4

lib_LTLIBRARIES        = libgraphlayout.la
libgraphlayout_la_LDFLAGS  = @GGOBI_LIBS@ $(LIBGVC_LIBS)
libgraphlayout_la_CFLAGS   = @GGOBI_CFLAGS@ @SRC_DEBUG@ $(LIBGVC_CFLAGS) -DHAVE_LIBGVC -Wno-unknown-pragmas
libgraphlayout_la_SOURCES  = cmds.c glayout.c graphviz.c init.c radial.c
