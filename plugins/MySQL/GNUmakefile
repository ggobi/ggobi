PKG=MySQL

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

-include local.config
-include $(GGOBI_HOME)/local.config

include ../GNUmakefile.plugin

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

MYSQL_INCLUDES=$(shell $(MYSQL_CONFIG) --cflags)
MYSQL_LIBS=$(shell $(MYSQL_CONFIG) --libs)

CFLAGS+=$(MYSQL_INCLUDES)
PLUGIN_LIBS+=$(MYSQL_LIBS)

SRC=mysqlInput
OBJS=$(SRC:%=%.o)

$(PKG).so: $(OBJS)

distclean::
	-rm -f local.config