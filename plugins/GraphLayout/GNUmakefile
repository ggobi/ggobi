PKG=GraphLayout

ifdef HAVE_LIBGVC
# For 2.3, graphviz uses pkgconfig and provides the interface in libgvc + gvc.h
 PLUGIN_CFLAGS+=-Wno-unknown-pragmas
 PLUGIN_CFLAGS+=$(LIBGVC_CFLAGS) -DHAVE_LIBGVC
 PLUGIN_LIBS+=$(LIBGVC_LIBS)
else
ifdef GRAPHVIZ

ifndef DOTNEATO_CONFIG
 DOTNEATO_CONFIG=dotneato-config
endif

ifndef GRAPHVIZ_INCLUDES
   GRAPHVIZ_INCLUDES= $(shell $(DOTNEATO_CONFIG) --cflags)
endif
# For 1.16, I need this plugin_libs piece.  For 1.12, I don't.
ifndef GRAPHVIZ_LIBS
 GRAPHVIZ_LIBS= $(shell $(DOTNEATO_CONFIG) --ldflags) $(shell $(DOTNEATO_CONFIG) --libs)
endif
 #CFLAGS+=-Wno-unknown-pragmas
 PLUGIN_CFLAGS+=-DGRAPHVIZ=1 $(GRAPHVIZ_INCLUDES)
 PLUGIN_LIBS+=$(GRAPHVIZ_LIBS)
endif
endif
