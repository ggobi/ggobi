DataViewer.so:

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

GTKEXTRA_INCLUDES=../../../gtk+extra-0.99.13/gtkextra
GTKEXTRA_LIB_DIR=/home2/FlowData/AltProjects/gtk+extra-0.99.13/gtkextra/.libs
EXTRA_LIBS=$(GTKEXTRA_LIB_DIR:%=-L%) $(GTKEXTRA_LIB_DIR:%=-Xlinker --rpath -Xlinker %) -lgtkextra

OBJS=worksheet.o

-include $(GGOBI_HOME)/local.config

DataViewer.so: $(OBJS)
	$(CC) -shared $(OBJS) -o $@ $(EXTRA_LIBS) `gtk-config --libs`

worksheet.o: worksheet.c
	$(CC) -g $(GTKEXTRA_INCLUDES:%=-I%) -I$(GGOBI_HOME) $(CFLAGS) `gtk-config --cflags` -o $@ -c $<
