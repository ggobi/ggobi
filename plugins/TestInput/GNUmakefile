-include local.config

GTK_CONFIG=gtk-config

GGOBI_INCLUDE_DIR=../..
GGOBI_LIB_DIR=../..

GGOBI_LIB=-L$(GGOBI_LIB_DIR) -lggobi
GGOBI_LIB=

CFLAGS+=-g -I$(GGOBI_INCLUDE_DIR) $(shell $(GTK_CONFIG) --cflags)

TestInput.so: testInput.o
	$(CC) -shared -o $@ testInput.o $(GGOBI_LIB)