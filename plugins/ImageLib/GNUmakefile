ifndef GGOBI_HOME
  GGOBI_HOME=../..
endif

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

-include local.config

ifndef IMLIB_CONFIG
 IMLIB_CONFIG=imlib-config
endif

CFLAGS=-g -I$(GGOBI_HOME) $(shell $(IMLIB_CONFIG) --cflags-gdk) $(shell gtk-config --cflags)
LIBS=$(shell $(IMLIB_CONFIG) --libs-gdk)

SRC=ImagePlugin
OBJS=$(SRC:%=%.o)

ImagePlugin.so: GNUmakefile $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(LIBS)
