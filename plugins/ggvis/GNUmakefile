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
GGOBI_LIB=

CFLAGS+=-g $(GGOBI_INCLUDE_DIR) $(shell $(GTK_CONFIG) --cflags) $(XML_INC_DIRS:%=-I%)

# used to comment out sections of code for incompletely
# implemented or buggy functionality
ifdef BINARY_IO_IMPLEMENTED
  CFLAGS+= -DBINARY_IO_IMPLEMENTED=1
endif
ifdef INFERENCE_IMPLEMENTED
  CFLAGS+= -DINFERENCE_IMPLEMENTED=1
endif
ifdef SMOOTH_IMPLEMENTED
  CFLAGS+= -DSMOOTH_IMPLEMENTED=1
endif
ifdef EDIT_EDGES_IMPLEMENTED
  CFLAGS+= -DEDIT_EDGES_IMPLEMENTED=1
endif
ifdef PRINTING_IMPLEMENTED
  CFLAGS+= -DPRINTING_IMPLEMENTED=1
endif
ifdef TS_EXTENSIONS_IMPLEMENTED
  CFLAGS+= -DTS_EXTENSIONS_IMPLEMENTED=1
endif
ifdef TOUR_PP_IMPLEMENTED
  CFLAGS+= -TOUR_PP_IMPLEMENTED=1
endif
ifdef TOUR_ADV_IMPLEMENTED
  CFLAGS+= -TOUR_ADV_IMPLEMENTED=1
endif
ifdef ROTATION_IMPLEMENTED
  CFLAGS+= -DROTATION_IMPLEMENTED=1
endif
ifdef DATAD_ADDED_SIGNAL_IMPLEMENTED
  CFLAGS+= -DDATAD_ADDED_SIGNAL_IMPLEMENTED
endif
ifdef COLORSCHEMES_IMPLEMENTED
  CFLAGS+= -DCOLORSCHEMES_IMPLEMENTED
endif
ifdef BARCHART_IMPLEMENTED
  CFLAGS+= -DBARCHART_IMPLEMENTED
endif

HEADERS=ggvis defines
SRC=ggvis init cmds spring radial
OBJS=$(SRC:%=%.o)

$(PKG).so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(GGOBI_LIB) $(shell $(GTK_CONFIG) --libs) $(XML_LIB_DIRS=%-L%)

local.config:  configure
	./configure

configure: configure.in
	autoconf

clean:
	-rm -f $(OBJS)

distclean: clean
	-rm -f $(PKG).so local.config
	-rm -f configure config.status config.cache config.log
