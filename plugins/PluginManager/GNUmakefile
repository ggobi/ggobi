PKG=PluginManager

$(PKG).so:

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif


-include local.config
-include $(GGOBI_HOME)/local.config

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

include $(GGOBI_HOME)/plugins/GNUmakefile.plugin

ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB=$(shell $(GGOBI_CONFIG) --libs)
else
 GGOBI_INCLUDE_DIR=-I../..
 GGOBI_LIB=-L../.. -lggobi
endif


CFLAGS+=$(shell cd $(GGOBI_HOME) ; make cflags)

CFLAGS+=$(GGOBI_INCLUDE_DIR) 


SRC=pluginViewer
OBJS=$(SRC:%=%.o)

$(PKG).so: $(OBJS)
	$(CC) -shared $(OBJS) -o $@ $(GGOBI_LIB)  $(EXTRA_LIBS)  


