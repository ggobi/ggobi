PKG=ggvis

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
 CFLAGS+=$(shell cd $(GGOBI_HOME) ; make cflags ${GGOBI_MAKEFILE} ) $(GGOBI_INCLUDE_DIR)
endif

HEADERS=ggvis
SRC=ggvis init ggv_cbacks stressplot histogram mds ggv_qsort \
 ggv_utils Dtarget anchor shepard
OBJS=$(SRC:%=%.o)

CFLAGS+=-g


ifdef WIN32
$(PKG).dll: $(OBJS) $(PKG).def
	$(CC) -mwindows -s -shared -o $@ $(PKG).def $(OBJS) $(GGOBI_LIB) $(LIBS) $(XML_LIBS)
else
$(PKG).so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(GGOBI_LIB) $(shell $(GTK_CONFIG) --libs) $(XML_LIB_DIRS=%-L%) $(LDFLAGS)
endif
local.config:  configure
	./configure

tmp:
	@echo "$(XML_LIBS)"
	@echo "$(LIBS)"

configure: configure.in
	autoconf

distclean: clean
	-rm -f $(PKG).so local.config
	-rm -f configure config.status config.cache config.log
