-include local.config

PG_INCLUDE_DIR=$(shell $(PG_CONFIG) --includedir)
PG_LIB_DIR=$(shell $(PG_CONFIG) --libdir)

ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB_DIR=$(shell $(GGOBI_CONFIG) --libs)
else
 GGOBI_INCLUDE_DIR=../..
 GGOBI_LIB_DIR=../..
endif

GGOBI_LIB=-L$(GGOBI_LIB_DIR) -lggobi
GGOBI_LIB=

CFLAGS+=-g -I$(PG_INCLUDE_DIR) -I$(GGOBI_INCLUDE_DIR) $(shell $(GTK_CONFIG) --cflags)

Postgres.so: pgInput.o
	$(CC) -shared -o $@ pgInput.o -L$(PG_LIB_DIR) -lpq $(GGOBI_LIB) $(shell $(GTK_CONFIG) --libs)


local.config:  configure
	./configure

configure: configure.in
	autoconf


clean:
	-rm pgInput.o

distclean: clean
	-rm Postgres.so local.config
	-rm configure config.status config.cache config.log