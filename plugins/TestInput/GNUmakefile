-include local.config

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB_DIR=$(shell $(GGOBI_CONFIG) --libs)
else
 GGOBI_INCLUDE_DIR=../..
 GGOBI_LIB_DIR=../..
endif

GGOBI_LIB=-L$(GGOBI_LIB_DIR) -lggobi
GGOBI_LIB=

CFLAGS+=-g -I$(GGOBI_INCLUDE_DIR) $(shell $(GTK_CONFIG) --cflags)

TestInput.so: testInput.o
	$(CC) -shared -o $@ testInput.o $(GGOBI_LIB)


local.config:  configure
	./configure

configure: configure.in
	autoconf


clean:
	-rm testInput.o

distclean: clean
	-rm TestInput.so local.config
	-rm configure config.status config.cache config.log