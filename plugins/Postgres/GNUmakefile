ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

-include local.config
-include $(GGOBI_HOME)/local.config

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

PG_INCLUDE_DIR=$(shell $(PG_CONFIG) --includedir)
PG_LIB_DIR=$(shell $(PG_CONFIG) --libdir)

ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB=$(shell $(GGOBI_CONFIG) --libs)
else
 GGOBI_INCLUDE_DIR=-I../..
 GGOBI_LIB=-L../.. -lggobi
endif


SRC=pgInput

CFLAGS+=-g -I$(PG_INCLUDE_DIR) $(GGOBI_INCLUDE_DIR) $(shell $(GTK_CONFIG) --cflags) $(XML_INC_DIRS:%=-I%)

Postgres.so: pgInput.o
	$(CC) -shared -o $@ pgInput.o -L$(PG_LIB_DIR) -lpq $(GGOBI_LIB) $(shell $(GTK_CONFIG) --libs) $(XML_LIB_DIRS:%=-L%)

pg: pgInput.c
	$(CC) -o $@ $(CFLAGS) -DSTANDALONE=1 pgInput.c -L$(PG_LIB_DIR) -lpq $(GGOBI_LIB) $(shell $(GTK_CONFIG) --libs) $(XML_LIB_DIRS:%=-L%)

local.config:  configure
	./configure

clean:
	-rm pgInput.o

distclean: clean
	-rm Postgres.so local.config
	-rm configure config.status config.cache config.log
