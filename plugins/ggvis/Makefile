ggvis.so:

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

# -rpath or --rpath?  The compiler on fry accepts -rpath.

include local.config
-include $(GGOBI_HOME)/local.config

OBJS=ggvis.o init.o

ggvis.so: $(OBJS)
	$(CC) -g -shared $(OBJS) -o $@ $(EXTRA_LIBS) -L$(GGOBI_HOME) -lggobi `gtk-config --libs` 

ggvis.o: ggvis.c
	$(CC) -g -I$(GGOBI_HOME) $(CFLAGS) `gtk-config --cflags` -o $@ -c $<
init.o: init.c
	$(CC) -g -I$(GGOBI_HOME) $(CFLAGS) `gtk-config --cflags` -o $@ -c $<

clean:
	rm -f *.o *.so
