DataViewer.so:

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

# -rpath or --rpath?  My compiler accepts -rpath.

EXTRA_LIBS=$(GTKEXTRA_LIB_DIR:%=-L%) $(GTKEXTRA_LIB_DIR:%=-Xlinker --rpath -Xlinker %) -lgtkextra

OBJS=worksheet.o

include local.config
-include $(GGOBI_HOME)/local.config

DataViewer.so: $(OBJS)
	$(CC) -shared $(OBJS) -o $@ $(EXTRA_LIBS) `gtk-config --libs`

worksheet.o: worksheet.c
	$(CC) -g $(GTKEXTRA_INCLUDES:%=-I%) -I$(GGOBI_HOME) $(CFLAGS) `gtk-config --cflags` -o $@ -c $<
