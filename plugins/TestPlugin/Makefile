TestPlugin.so:

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

# -rpath or --rpath?  The compiler on fry accepts -rpath.

EXTRA_LIBS=

SRC=testplugin
OBJS=testplugin.o

include local.config
-include $(GGOBI_HOME)/local.config

ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB=$(shell $(GGOBI_CONFIG) --libs)
else
 GGOBI_INCLUDE_DIR=-I../..
 GGOBI_LIB=-L../.. -lggobi
endif

CFLAGS+= $(shell $(GTK_CONFIG) --cflags)
ifdef  EXTRA_INCLUDE_DIR
 CFLAGS+=$(EXTRA_INCLUDE_DIR:%=-I%)
endif


GTK_LIBS=$(shell $(GTK_CONFIG) --libs)

TestPlugin.so: $(OBJS)
	$(CC) -shared $(OBJS) -o $@ $(EXTRA_LIBS) $(GGOBI_LIB) $(GTK_LIBS)

testplugin.o: testplugin.c
	$(CC) -g $(GGOBI_INCLUDE_DIR) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.so

local.config: local.config.in configure
	./configure

configure: configure.in 
	autoconf
