DataViewer.so:

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

# -rpath or --rpath?  The compiler on fry accepts -rpath.

include local.config
-include $(GGOBI_HOME)/local.config

ifdef GTKEXTRA_CONFIG
 GTKEXTRA_INCLUDES=$(shell $(GTKEXTRA_CONFIG) --cflags)
 EXTRA_LIBS=$(shell $(GTKEXTRA_CONFIG) --libs)
else
 EXTRA_LIBS=$(GTKEXTRA_LIB_DIR:%=-L%) $(GTKEXTRA_LIB_DIR:%=-Xlinker -rpath -Xlinker %) -lgtkextra 
endif

OBJS=worksheet.o

DataViewer.so: $(OBJS)
	$(CC) -shared $(OBJS) -o $@ $(EXTRA_LIBS) -L$(GGOBI_HOME) -lggobi `gtk-config --libs` 

worksheet.o: worksheet.c
	$(CC) -g $(GTKEXTRA_INCLUDES) -I$(GGOBI_HOME) $(CFLAGS) `gtk-config --cflags` -o $@ -c $<

clean:
	rm -f *.o *.so
