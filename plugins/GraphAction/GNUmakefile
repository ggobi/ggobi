PKG=GraphAction

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

-include local.config
-include $(GGOBI_HOME)/local.config

ifdef WIN32
  GGOBI_MAKEFILE=-f Makefile.win32
endif


ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB=$(shell $(GGOBI_CONFIG) --libs)
 CFLAGS+=$(shell $(GGOBI_CONFIG) --cflags) $(GGOBI_INCLUDE_DIR)
else
 GGOBI_INCLUDE_DIR=-I../..
 GGOBI_LIB=-L../.. -lggobi
 CFLAGS+=$(shell cd $(GGOBI_HOME) ; make cflags $(GGOBI_MAKEFILE)) $(GGOBI_INCLUDE_DIR)
endif

#GGOBI_LIB=-L$(GGOBI_LIB_DIR) -lggobi
#GGOBI_LIB=

HEADERS=graphact
SRC=graphact init ga_cbacks
OBJS=$(SRC:%=%.o)

ifdef WIN32
$(PKG).dll: $(OBJS) $(PKG).def
	$(CC) -mwindows -s -shared -o $@ $(PKG).def $(OBJS) $(GGOBI_LIB) $(LIBS) $(XML_LIBS)
else
$(PKG).so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(GGOBI_LIB) $(shell $(GTK_CONFIG) --libs) $(XML_LIB_DIRS=%-L%) $(LDFLAGS)
endif

local.config:  configure
	./configure

ifndef ADMIN
configure: configure.in
	autoconf
endif


distclean::
	-rm -f local.config

tmp:
	@echo "$(XML_LIBS)"
	@echo "$(LIBS)"
