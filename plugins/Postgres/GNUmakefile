-include local.config

PG_INCLUDE_DIR=$(PG_ROOT)/include
PG_LIB_DIR=$(PG_ROOT)/lib
GGOBI_INCLUDE_DIR=../..
GGOBI_LIB_DIR=../..

GGOBI_LIB=-L$(GGOBI_LIB_DIR) -lggobi
GGOBI_LIB=

GTK_CONFIG=gtk-config

CFLAGS+=-g -I$(PG_INCLUDE_DIR) -I$(GGOBI_INCLUDE_DIR) $(shell $(GTK_CONFIG) --cflags)

Postgres.so: pgInput.o
	$(CC) -shared -o $@ pgInput.o -L$(PG_LIB_DIR) -lpq $(GGOBI_LIB) $(shell $(GTK_CONFIG) --libs)
