PKG=ggvis

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
else
 GGOBI_INCLUDE_DIR=-I../..
 GGOBI_LIB=-L../.. -lggobi
endif

GGOBI_LIB=-L$(GGOBI_LIB_DIR) -lggobi
#GGOBI_LIB=

CFLAGS+=$(shell cd $(GGOBI_HOME) ; make cflags) $(GGOBI_INCLUDE_DIR)

ifdef GRAPHVIZ
ARCH=linux.i386
GVINC=-I$(GVIZ)/include -I$(GVIZ_SRC) -I$(GVIZ_SRC)/dotneato/common
GVLIB=-L$(GVIZ)/build/$(ARCH)/lib

EXTLIB_INC=-I/usr/include/freetype2
EXTLIB_LIB=-ljpeg -lpng -lfreetype -lz

CFLAGS+= -DGRAPHVIZ=1 $(GVINC)
LDFLAGS+=$(GVLIB) -ldot -lneato -lpack -ldotneato -lgraph -lpathplan -lgd -lcdt $(EXTLIB_LIB)
endif

ifdef BINARY_IO_IMPLEMENTED
  CFLAGS+= -DBINARY_IO_IMPLEMENTED=1
endif
ifdef INFERENCE_IMPLEMENTED
  CFLAGS+= -DINFERENCE_IMPLEMENTED=1
endif
ifdef SMOOTH_IMPLEMENTED
  CFLAGS+= -DSMOOTH_IMPLEMENTED=1
endif
ifdef TS_EXTENSIONS_IMPLEMENTED
  CFLAGS+= -DTS_EXTENSIONS_IMPLEMENTED=1
endif
ifdef BARCHART_IMPLEMENTED
  CFLAGS+= -DBARCHART_IMPLEMENTED
endif

HEADERS=ggvis defines
SRC=ggvis init cmds spring radial graphviz
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
