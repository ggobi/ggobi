ggvis.so:

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

# -rpath or --rpath?  The compiler on fry accepts -rpath.

-include local.config
-include $(GGOBI_HOME)/local.config

OBJS=ggvis.o init.o cmds.o spring.o radial.o

ggvis.so: $(OBJS)
	$(CC) -g -shared $(OBJS) -o $@ $(EXTRA_LIBS)  -L$(GGOBI_HOME) -lggobi `gtk-config --libs` 

.c.o:
	$(CC) -g -c -I. -I$(GGOBI_HOME) $(CFLAGS) $(XML_INC_DIRS) `gtk-config --cflags`  $*.c

clean:
	rm -f *.o *.so


