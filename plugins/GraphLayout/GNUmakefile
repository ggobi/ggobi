PKG=GraphLayout

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

-include local.config
-include $(GGOBI_HOME)/local.config

ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB=$(shell $(GGOBI_CONFIG) --libs)
 CFLAGS+=$(shell $(GGOBI_CONFIG) --cflags) $(GGOBI_INCLUDE_DIR)
else
 GGOBI_INCLUDE_DIR=-I../..
 GGOBI_LIB=-L../.. -lggobi
 CFLAGS+=$(shell cd $(GGOBI_HOME) ; make cflags) $(GGOBI_INCLUDE_DIR)
endif

GGOBI_LIB=-L$(GGOBI_LIB_DIR) -lggobi
GGOBI_LIB=

ifdef GRAPHVIZ
CFLAGS+=-Wno-unknown-pragmas
CFLAGS+= -DGRAPHVIZ=1 $(GVINC:%=-I%)
LDFLAGS+=-L$(GVLIB) -ldot -lneato -lpack -ldotneato -lgraph -lpathplan -lgd -lcdt -ltcldot
endif

HEADERS=glayout defines
SRC=glayout init radial graphviz
OBJS=$(SRC:%=%.o)

$(PKG).so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(GGOBI_LIB) $(shell $(GTK_CONFIG) --libs) $(XML_LIB_DIRS=%-L%) $(LDFLAGS)

local.config:  configure
	./configure

configure: configure.in
	autoconf

distclean: clean
	-rm -f $(PKG).so local.config
	-rm -f configure config.status config.cache config.log
