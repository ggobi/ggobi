ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

-include local.config
-include $(GGOBI_HOME)/local.config

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB=$(shell $(GGOBI_CONFIG) --libs)
else
 GGOBI_INCLUDE_DIR=-I../..
 GGOBI_LIB=-L../.. -lggobi
endif

SRC=testInput

CFLAGS+=-g $(GGOBI_INCLUDE_DIR) $(shell $(GTK_CONFIG) --cflags) $(XML_INC_DIRS:%=-I%)

TestInput.so: testInput.o
	$(CC) -shared -o $@ testInput.o $(GGOBI_LIB) $(XML_LIB_DIRS=%-L%)


local.config:  configure
	./configure

clean:
	-rm testInput.o

distclean: clean
	-rm TestInput.so local.config
	-rm configure config.status config.cache config.log
