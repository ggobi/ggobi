CategoricalPlots.so:

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

# -rpath or --rpath?  The compiler on fry accepts -rpath.

-include $(GGOBI_HOME)/local.config
-include local.config


# On some platforms, we will need to link the plugin against libggobi.so
# and also the ggobi executable in order to resolve symbols referenced
# in the plugin and supplied/located in the executable itself.
# This becomes necessary when the plugins are used in a ggobi that is itself
# embedded (e.g. R)
GGOBI_LIB=-L$(GGOBI_HOME) -lggobi

OBJS=categoricalPlots.o

CategoricalPlots.so: $(OBJS)
	$(CC) -g -shared $(OBJS) -o $@ $(GGOBI_LIB) `gtk-config --libs` 

%.o: %.c
	$(CC) -g -I$(GGOBI_HOME) $(CFLAGS) `gtk-config --cflags` -o $@ -c $<

clean:
	rm -f *.o *.so
