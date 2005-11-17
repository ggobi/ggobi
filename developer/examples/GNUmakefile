

ifdef FOOEY
GGOBI_CONFIG=ggobi-config

CFLAGS=$(shell $(GGOBI_CONFIG) --cflags)
LIBS=$(shell $(GGOBI_CONFIG) --libs)

else

CFLAGS=-I.. $(shell $(MAKE) --quiet -C .. cflags)
LIBS=-L../ -lggobi $(shell $(GTK_CONFIG) --libs)

endif

layout: layout.o
	$(CC) -o $@ $^ $(LIBS)

mygobi: mygobi.o
	$(CC) -o $@ $^ $(LIBS)
