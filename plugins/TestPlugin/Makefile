TestPlugin.so:

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

# -rpath or --rpath?  The compiler on fry accepts -rpath.

EXTRA_LIBS=

OBJS=testplugin.o

include local.config
-include $(GGOBI_HOME)/local.config

CFLAGS+= $(shell $(GTK_CONFIG) --cflags)
ifdef  EXTRA_INCLUDE_DIR
 CFLAGS+=$(EXTRA_INCLUDE_DIR:%=-I%)
endif


GTK_LIBS=$(shell $(GTK_CONFIG) --libs)

TestPlugin.so: $(OBJS)
	$(CC) -shared $(OBJS) -o $@ $(EXTRA_LIBS) $(GTK_LIBS)

testplugin.o: testplugin.c
	$(CC) -g -I$(GGOBI_HOME) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.so

local.config: local.config.in configure
	./configure

configure: configure.in 
	autoconf
