PKG=FileSelect

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

ifdef ADMIN
 include Install/GNUmakefile.admin
else
-include local.config
endif

-include $(GGOBI_HOME)/local.config

GTK_EXTRA_INCLUDE_DIR=$(shell $(GTK_EXTRA_CONFIG) --cflags)
GTK_EXTRA_LIB_DIR=$(shell $(GTK_EXTRA_CONFIG) --libs)

ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB=$(shell $(GGOBI_CONFIG) --libs)
else
 GGOBI_INCLUDE_DIR=-I../..
 GGOBI_LIB=-L../.. -lggobi
endif

GGOBI_LIB=-L$(GGOBI_LIB_DIR) -lggobi
GGOBI_LIB=

CFLAGS+=-g $(GTK_EXTRA_INCLUDE_DIR) $(GGOBI_INCLUDE_DIR) $(shell $(GTK_CONFIG) --cflags) $(XML_INC_DIRS:%=-I%)

SRC=fileSelect
OBJS=$(SRC:%=%.o)

$(PKG).so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(GTK_EXTRA_LIB_DIR) $(GGOBI_LIB) $(shell $(GTK_CONFIG) --libs) $(XML_LIB_DIRS=%-L%)

local.config:  configure
	./configure

configure: configure.in
	autoconf

clean:
	-rm -f $(OBJS)

distclean: clean
	-rm -f $(PKG).so local.config
	-rm -f configure config.status config.cache config.log
