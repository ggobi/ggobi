ggobi:

local.config: developerConfigure
	./developerConfigure
	@echo "Created local.config via developerConfigure"

include local.config

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

ggobi:

ifndef CC
CC = gcc
#CC = cc
endif


# This defaults to $(CC) and is reset to CXX by any optional 
# segment that needs to use C++, e.g  USE_MYSQL 
LD=$(CXX)
LD=$(CC)


CFLAGS+= -g -g2 -Wall -fPIC -DHAVE_GGOBI_CONFIG_H -DPARCOORDS_DRAG_AND_DROP=1  $(EXTRA_CFLAGS)
#CFLAGS= -g -w -DHAVE_CONFIG_H # when using Irix cc, suppress warnings
CXXFLAGS=$(CFLAGS)

ifndef GTK_CONFIG
 GTK_CONFIG=gtk-config
endif

ifndef GTK_CFLAGS
 GTK_CFLAGS=`$(GTK_CONFIG) --cflags`
endif

ifndef GTK_LIBS
 GTK_LIBS=`$(GTK_CONFIG) --libs`
endif

ifdef TEST_KEYS
 CFLAGS+= -DTEST_KEYS=1
endif

ifdef TESTING_ROWS_IN_PLOT_CB
 CFLAGS+= -DTESTING_ROWS_IN_PLOT_CB=0
endif

ifdef TESTING_TOUR_STEP
 CFLAGS+= -DTESTING_TOUR_STEP=1
endif

ifndef DOXYGEN
  DOXYGEN=doxygen
endif


ifdef GTK_2
GLIB_GENMARSHAL=glib-genmarshal

marshal.h: marshal.list
	 $(GLIB_GENMARSHAL) --prefix=gtk_marshal $< --header > $@

marshal.c: marshal.list
	 $(GLIB_GENMARSHAL) --prefix=gtk_marshal $< --body > $@

make_ggobi.o: marshal.h GtkSignalDef.c

ggobiClass.o: marshal.h
endif

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

SHLIB_LDFLAGS= -shared
ifndef SHARED_LD_FLAGS
  SHARED_LD_FLAGS= -shared
endif
LDFLAGS=

SRC=\
 GGobiAppClass.c \
 array.c ash1d.c \
 barchart.c barchart_ui.c barchartClass.c \
 brush_api.c brush_bins.c brush.c brush_init.c brush_link.c brush_ui.c \
 color.c color_ui.c cpanel.c \
 datad.c display.c display_tree.c display_ui.c \
 edges.c exclusion.c exclusion_ui.c extendedDisplay.c fileio.c \
 ggobi-API.c ggobi.c ggobiClass.c \
 help.c identify.c identify_ui.c \
 impute.c impute_ui.c io.c jitter.c jitter_ui.c \
 limits.c lineedit.c lineedit_ui.c \
 main_ui.c make_ggobi.c menus.c missing.c \
 movepts.c movepts_ui.c noop-toggle.c \
 p1d.c p1d_ui.c \
 parcoords.c parcoords_ui.c parcoordsClass.c pipeline.c plugin.c \
 ppcorr_ui.c \
 read_array.c read_color.c read_data.c read_init.c read_xml.c \
 read_excel.c rb.c \
 scale_api.c scale_click.c scale_drag.c scale_ui.c \
 scatmat.c scatmat_ui.c scatmatClass.c \
 scatterplot.c scatterplot_ui.c scatterplotClass.c \
 smooth_ui.c sphere.c sphere_ui.c splash.c \
 splot.c sp_plot.c sp_plot_axes.c sp_plot_edges.c subset.c subset_ui.c svd.c \
 texture.c timeplot.c time_ui.c tsdisplay.c tsPlot.c \
 tour1d.c tour1d_pp.c tour1d_pp_ui.c tour1d_ui.c tour_pp.c\
 tour2d.c tour2d_ui.c tour2d_pp.c tour2d_pp_ui.c \
 tour2d3.c tour2d3_ui.c \
 tour.c tourcorr.c tourcorr_ui.c \
 transform.c transform_ui.c \
 utils.c utils_gdk.c utils_ui.c \
 varchange.c varcircles.c varpanel_ui.c \
 vartable.c vartable_nbook.c vartable_ui.c \
 vector.c wvis.c wvis_ui.c win32_draw.c \
 writedata.c writedata_ui.c write_state.c write_svg.c write_xml.c \
 xlines.c xyplot.c xyplot_ui.c \
\
 mt19937ar.c cokus.c \
 print.c 


ifdef GTK_2
 SRC+=marshal.c
else
 SRC+=gtkextruler.c gtkexthruler.c gtkextvruler.c 
endif

ifdef TEST_EVENTS
  SRC+=  testEvents.c
  CFLAGS+= -DTEST_GGOBI_EVENTS -DTEST_BRUSH_MOTION_CB=1 -DCHECK_EVENT_SIGNATURES=1
endif

# XML_FLAGS+= -DSUPPORT_PLUGINS=1 -DSUPPORT_INIT_FILES=1
CFLAGS+= $(XML_INC_DIRS:%=-I%) $(XML_FLAGS) -DSUPPORT_PLUGINS=1 -DSUPPORT_INIT_FILES=1
CFLAGS+=$(DEFINES)

ifdef DL_RESOLVE_FLAG
 DL_RESOLVE_PATH+=$(XML_LIB_DIRS:%=$(DL_RESOLVE_FLAG) %)
endif
XML_LIB_NO=2

ifndef XML_LIBS
  XML_LIBS=$(XML_LIB_DIRS:%=-L%) -lxml$(XML_LIB_NO) -lz 
endif


main_ui.o: write_xml.h
read_xml.o: read_xml.h

ifdef USE_DBMS
 SRC+= dbms.c dbms_ui.c
endif

# After all appropriate optional files have been appended to
OB=$(SRC:%.c=%.o)

ggobi: $(OB) $(EXTRA_OB) ggobiMain.o
	$(LD) ggobiMain.o $(OB) $(EXTRA_OB) $(LDFLAGS) -o ggobi $(XML_LIBS) $(MYSQL_LIBS)  $(EXTRA_LIBS) ${GTK_LIBS}  $(DL_RESOLVE_PATH)

pure: ggobi.o $(OB) $(EXTRA_OB)
	purify -cache-dir=/usr/dfs/tmp -always-use-cache-dir=yes \
	-user-path=/usr/dfs/ggobi/ggobi:/usr/dfs/ggobi/ggobi/plugins/GraphLayout \
	$(LD) $(OB) $(EXTRA_OB) \
	/usr/dfs/ggobi/ggobi/plugins/GraphLayout/glayout.o \
	/usr/dfs/ggobi/ggobi/plugins/GraphLayout/graphviz.o \
	/usr/dfs/ggobi/ggobi/plugins/GraphLayout/init.o \
	/usr/dfs/ggobi/ggobi/plugins/GraphLayout/radial.o \
	$(LDFLAGS) -o ggobi \
	/usr/dfs/cc/lib/libxml2.so \
	`gtk-config --libs`

%.sched: %.c
	$(CC) -dS -c $(CFLAGS) -I. $(GTK_CFLAGS) $*.c

ggobi.sched: $(OB)
	$(CC) -S $(OB) $(LDFLAGS)  $(XML_LIB_DIRS:%=-L%) $(XML_LIBS) `gtk-config --cflags --libs`

mt19937ar.o: mt19937ar.c
	$(CC) $(CFLAGS) `gtk-config --cflags` -c $<

efence: $(OB)
	MALLOC_CHECK_=2
	$(CC) $(OB) $(LDFLAGS) $(EFENCE_LIBS) $(XML_LIB_DIRS:%=-L%) $(XML_LIBS) $(GTK_LIBS)

lib: libggobi.so
libggobi.so: $(OB)
	$(CC) -g $(SHARED_LD_FLAGS) -o $@ $(OB) $(XML_LIB_DIRS:%=-L%) $(XML_LIBS) $(GTK_LIBS) $(DL_RESOLVE_PATH)


ggobi-API.o ggobi.o: config.h

config.h: developerConfigure
	./developerConfigure

developerConfigure: developerConfigure.in
	autoconf $< > $@
	chmod +x $@


clean: 
	rm -f *.o ggobi libggobi.so

.c.o:
	$(CC) -c -I. $(CFLAGS) $(GTK_CFLAGS)  $*.c

# A version that compiles all C code (except for mt19937ar.c) as 
# C++.
#	$(CXX) -c $(CXXFLAGS) -I. `gtk-config --cflags` $*.c


ifdef DEPENDS_FLAG
depends: $(SRC)
	$(CC) $(DEPENDS_FLAG) $(CFLAGS) -I. $(GTK_CFLAGS) $(SRC) > $@	

include depends

endif


readXML: read_xml.o read_data.o fileio.o read_color.o read_init.o plugin.o brush_init.o ggobi.o
	$(CC) -o $@ $^ $(XML_LIB_DIRS:%=-L%) $(XML_LIBS) -lgtk -lgdk

xmlConvert: xmlConvert.o libggobi.so
	$(CC) -g -o $@ xmlConvert.o $(XML_LIBS) $(XML_LIB_DIRS:%=-L%) -L. -lggobi $(DL_RESOLVE_PATH) -lgtk

make_ggobi.o: read_xml.h

dbms_ui.o: dbms_ui.c dbms_ui.h dbms.h
dbms.c: dbms.h  ggobi.h


ifdef USE_PROPERTIES
dbms_ui.o: dbms_ui.c dbms_ui.h
	$(CXX) `gtk-config --cflags` $(CFLAGS) -c $<
endif


# Emacs's tags for navigating through the source.
TAGS etags: $(SRC)
	etags $(SRC)

# The version for vi
tags:
	tags $(SRC)


datad.o read_xml.o: datad.c datad.h


../bin/ggobi: ggobi
	cp ggobi $@

PWD=$(shell pwd)

cflags:
	@echo "-I$(PWD) $(CFLAGS) $(GTK_CFLAGS)"

libs:
	@echo "-L$(PWD) -lggobi $(GTK_LIBS)"

print.o: print.c print.h

%.o: %.d

%.d: %.c
	$(CC) -M $(CFLAGS) -I. $(GTK_CFLAGS) $< > $@

%.e: %.c
	$(CC) -E $(CFLAGS) -I. $(GTK_CFLAGS) $< > $@


ifndef R
  # how to invoke R.
 R=R
endif

USE_R_XSL=1

ifndef XSLT
 XSLT=xsltproc
endif

help.o: CmdArgHelp.c

# One can also use the XSL mechanism
# as in
CmdArgHelp.c: share/R/commandArgs.S Docs/commandArgs.xml
ifdef USE_R_XSL
	echo 'source("share/R/commandArgs.S") ; cat(createCmdArgHelp(getArgInfo("Docs/commandArgs.xml")), "\\\n", file="$@")' | $(R) --vanilla 
else
	 $(XSLT) share/XSL/CmdArgHelp.xsl Docs/commandArgs.xml > $@
endif

# Where is the output?
apiDoc: Install/apiDocConfig
	$(DOXYGEN) Install/apiDocConfig

testEvents.o: GGobiEvents.h

# DO NOT DELETE

