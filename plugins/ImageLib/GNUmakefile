ifndef GGOBI_HOME
  GGOBI_HOME=../..
endif

CFLAGS=-g -I$(GGOBI_HOME) $(shell imlib-config --cflags-gdk) $(shell gtk-config --cflags)
LIBS=$(shell imlib-config --libs-gdk)

SRC=ImagePlugin.c
OBJS=$(SRC:%.c=%.o)

ImagePlugin.so: GNUmakefile $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(LIBS)
