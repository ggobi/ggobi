TestPlugin.so:

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

# -rpath or --rpath?  The compiler on fry accepts -rpath.

EXTRA_LIBS=

OBJS=testplugin.o

include local.config
-include $(GGOBI_HOME)/local.config

TestPlugin.so: $(OBJS)
	$(CC) -shared $(OBJS) -o $@ $(EXTRA_LIBS) `gtk-config --libs`

testplugin.o: testplugin.c
	$(CC) -g $(GTKEXTRA_INCLUDES:%=-I%) -I$(GGOBI_HOME) $(CFLAGS) `gtk-config --cflags` -o $@ -c $<

clean:
	rm -f *.o *.so
