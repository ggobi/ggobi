ggvis.so:

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

# -rpath or --rpath?  The compiler on fry accepts -rpath.

-include local.config
-include $(GGOBI_HOME)/local.config

# used to comment out sections of code for incompletely
# implemented or buggy functionality
ifdef BINARY_IO_IMPLEMENTED
  CFLAGS+= -DBINARY_IO_IMPLEMENTED=1
endif
ifdef INFERENCE_IMPLEMENTED
  CFLAGS+= -DINFERENCE_IMPLEMENTED=1
endif
ifdef SMOOTH_IMPLEMENTED
  CFLAGS+= -DSMOOTH_IMPLEMENTED=1
endif
ifdef EDIT_EDGES_IMPLEMENTED
  CFLAGS+= -DEDIT_EDGES_IMPLEMENTED=1
endif
ifdef PRINTING_IMPLEMENTED
  CFLAGS+= -DPRINTING_IMPLEMENTED=1
endif
ifdef TS_EXTENSIONS_IMPLEMENTED
  CFLAGS+= -DTS_EXTENSIONS_IMPLEMENTED=1
endif
ifdef TOUR_PP_IMPLEMENTED
  CFLAGS+= -TOUR_PP_IMPLEMENTED=1
endif
ifdef TOUR_ADV_IMPLEMENTED
  CFLAGS+= -TOUR_ADV_IMPLEMENTED=1
endif
ifdef ROTATION_IMPLEMENTED
  CFLAGS+= -DROTATION_IMPLEMENTED=1
endif
ifdef DATAD_ADDED_SIGNAL_IMPLEMENTED
  CFLAGS+= -DDATAD_ADDED_SIGNAL_IMPLEMENTED
endif
ifdef COLORSCHEMES_IMPLEMENTED
  CFLAGS+= -DCOLORSCHEMES_IMPLEMENTED
endif
ifdef BARCHART_IMPLEMENTED
  CFLAGS+= -DBARCHART_IMPLEMENTED
endif

OBJS=ggvis.o init.o cmds.o spring.o radial.o

ggvis.so: $(OBJS)
	$(CC) -g -shared $(OBJS) -o $@ $(EXTRA_LIBS)  -L$(GGOBI_HOME) -lggobi `gtk-config --libs` 

.c.o:
	$(CC) -g -c -I. -I$(GGOBI_HOME) $(CFLAGS) $(XML_INC_DIRS:%=-I%) `gtk-config --cflags`  $*.c

clean:
	rm -f *.o *.so


