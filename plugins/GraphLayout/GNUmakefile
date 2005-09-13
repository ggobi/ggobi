PKG=GraphLayout

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

-include local.config
-include $(GGOBI_HOME)/local.config

include ../GNUmakefile.plugin

ifdef WIN32
  GGOBI_MAKEFILE=-f Makefile.win32
endif

ifndef GTK_CONFIG
 GTK_CONFIG=gtk-config
endif

ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB=$(shell $(GGOBI_CONFIG) --libs)
 CFLAGS+=$(shell $(GGOBI_CONFIG) --cflags) $(GGOBI_INCLUDE_DIR)
else
 GGOBI_INCLUDE_DIR=-I../..
 GGOBI_LIB=-L../.. -lggobi
 CFLAGS+=$(shell cd $(GGOBI_HOME) ; $(MAKE) cflags $(GGOBI_MAKEFILE)) $(GGOBI_INCLUDE_DIR)
endif

ifdef HAVE_LIBGVC
# For 2.3, graphviz uses pkgconfig and provides the interface in libgvc + gvc.h
 CFLAGS+=-Wno-unknown-pragmas
 CFLAGS+=$(LIBGVC_CFLAGS) -DHAVE_LIBGVC
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
 CFLAGS+=-DGRAPHVIZ=1 $(GRAPHVIZ_INCLUDES)
 PLUGIN_LIBS+=$(GRAPHVIZ_LIBS)
endif
endif


ifdef WITH_R
 CFLAGS+= -I$(OMEGA_HOME)/include/Corba -I$(R_HOME)/include -DWITH_R=1 -DUSE_R=1 -D_R_=1
endif

HEADERS=glayout defines
SRC=glayout init radial graphviz
OBJS=$(SRC:%=%.o)

ifdef WIN32
$(PKG).dll: $(OBJS) $(PKG).def
	$(CC) -mwindows -s -shared -o $@ $(PKG).def $(OBJS) $(GGOBI_LIB) $(LIBS) $(XML_LIBS)
#else
#$(PKG).so: $(OBJS)
#	$(CC) -shared -o $@ $(OBJS) $(GGOBI_LIB) $(shell $(GTK_CONFIG) --libs) $(XML_LIB_DIRS=%-L%) $(LDFLAGS)
endif

$(PKG).so: $(OBJS)

distclean::
	-rm -f local.config

