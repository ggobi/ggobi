-include local.config

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

CFLAGS+=-g -I$(PG_INCLUDE_DIR) $(GGOBI_INCLUDE_DIR) $(shell $(GTK_CONFIG) --cflags)

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