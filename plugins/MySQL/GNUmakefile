PKG=MySQL

ifdef ADMIN
 include Install/GNUmakefile.admin
endif


-include local.config

MYSQL_INCLUDE_DIR=$(shell $(MYSQL_CONFIG) --cflags)
MYSQL_LIB_DIR=$(shell $(MYSQL_CONFIG) --libs)

ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB=$(shell $(GGOBI_CONFIG) --libs)
else
 GGOBI_INCLUDE_DIR=-I../..
 GGOBI_LIB=-L../.. -lggobi
endif

CFLAGS+=-g $(MYSQL_INCLUDE_DIR) $(GGOBI_INCLUDE_DIR) $(shell $(GTK_CONFIG) --cflags)

SRC=mysqlInput
OBJS=$(SRC:%=%.o)

$(PKG).so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(MYSQL_LIB_DIR) $(GGOBI_LIB) $(shell $(GTK_CONFIG) --libs)

local.config:  configure
	./configure

configure: configure.in
	autoconf

clean:
	-rm $(OBJS)

distclean: clean
	-rm $(PKG).so local.config
	-rm configure config.status config.cache config.log