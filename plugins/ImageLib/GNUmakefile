PKG=ImageLib

ifndef GGOBI_HOME
  GGOBI_HOME=../..
endif

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

-include $(GGOBI_HOME)/local.config

# include ../GNUmakefile.plugin

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

ifndef IMLIB_CONFIG
 IMLIB_CONFIG=imlib-config
endif

CFLAGS=-g -I$(GGOBI_HOME) $(shell $(IMLIB_CONFIG) --cflags-gdk) $(shell gtk-config --cflags) $(XML_FLAGS)
LIBS=$(shell $(IMLIB_CONFIG) --libs-gdk) $(XML_LIBS)

SRC=ImagePlugin
OBJS=$(SRC:%=%.o)

$(PKG).so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(LIBS) $(GGOBI_LIB) 

clean:
	@rm -f *.o $(PKG).so

test:
	@echo IMLIB_CONFIG=$(IMLIB_CONFIG)
	@echo CFLAGS=$(CFLAGS)
