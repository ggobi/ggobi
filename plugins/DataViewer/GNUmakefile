PKG=DataViewer

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

ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB=$(shell $(GGOBI_CONFIG) --libs)
 CFLAGS+=$(shell $(GGOBI_CONFIG) --cflags) $(GGOBI_INCLUDE_DIR)
else
 GGOBI_INCLUDE_DIR=-I../..
 GGOBI_LIB=-L../.. -lggobi
 CFLAGS+=$(shell cd $(GGOBI_HOME) ; $(MAKE) cflags $(GGOBI_MAKEFILE)) $(GGOBI_INCLUDE_DIR)
endif

ifdef WITH_R
 CFLAGS+= -I$(OMEGA_HOME)/include/Corba -I$(R_HOME)/include -DWITH_R=1 -DUSE_R=1 -D_R_=1
endif

SRC=worksheet
OBJS=$(SRC:%=%.o)

ifdef WIN32
$(PKG).dll: $(OBJS) $(PKG).def
	$(CC) -mwindows -s -shared -o $@ $(PKG).def $(OBJS) $(GGOBI_LIB) $(LIBS) $(XML_LIBS)
#else
#$(PKG).so: $(OBJS)
#	$(CC) -shared -o $@ $(OBJS) $(GGOBI_LIB) $(shell $(GTK_CONFIG) --libs) $(XML_LIB_DIRS=%-L%) $(LDFLAGS)
endif

test:
	@echo CFLAGS=$(CFLAGS)

$(PKG).so: $(OBJS)

distclean::
	-rm -f local.config

