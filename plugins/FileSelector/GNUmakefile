PKG=FileSelect

-include local.config

GTK_EXTRA_INCLUDE_DIR=$(shell $(GTK_EXTRA_CONFIG) --cflags)
GTK_EXTRA_LIB_DIR=$(shell $(GTK_EXTRA_CONFIG) --libs)

ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB_DIR=$(shell $(GGOBI_CONFIG) --libs)
else
 GGOBI_INCLUDE_DIR=../..
 GGOBI_LIB_DIR=../..
endif

GGOBI_LIB=-L$(GGOBI_LIB_DIR) -lggobi
GGOBI_LIB=

CFLAGS+=-g $(GTK_EXTRA_INCLUDE_DIR) -I$(GGOBI_INCLUDE_DIR) $(shell $(GTK_CONFIG) --cflags)

SRC=fileSelect
OBJS=$(SRC:%=%.o)

$(PKG).so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(GTK_EXTRA_LIB_DIR) $(GGOBI_LIB) $(shell $(GTK_CONFIG) --libs)

local.config:  configure
	./configure

configure: configure.in
	autoconf

clean:
	-rm $(OBJS)

distclean: clean
	-rm $(PKG).so local.config
	-rm configure config.status config.cache config.log