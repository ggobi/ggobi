ifdef ADMIN
 include Install/GNUmakefile.admin
endif

ggobi:

include local.config

CC = gcc

# This defaults to $(CC) and is reset to CXX by any optional 
# segment that needs to use C++, e.g  USE_MYSQL 
LD=$(CXX)
LD=$(CC)

CFLAGS= -g2 -ansi -Wall -fpic -DHAVE_CONFIG_H
#CFLAGS= -g2 -ansi -DHAVE_CONFIG_H  # when using Irix cc
CXXFLAGS=$(CFLAGS)

ifdef TEST_KEYS
 CFLAGS+= -DTEST_KEYS=1
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

SHLIB_LDFLAGS= -shared
SHARED_LD_FLAGS= -shared
LDFLAGS=

SRC=ggobi.c datad.c make_ggobi.c color.c main_ui.c splash.c cpanel.c \
 menus.c \
 utils.c utils_ui.c utils_gdk.c array.c vector.c \
 read_array.c read_data.c io.c writedata_ui.c writedata.c write_svg.c \
 limits.c pipeline.c missing.c \
 scatterplot.c scatterplot_ui.c \
 splot.c sp_plot.c win32_draw.c \
 display.c display_ui.c \
 p1d_ui.c p1d.c ash1d.c texture.c \
 xyplot_ui.c xyplot.c \
 rotate_ui.c \
 tour1d_ui.c tour1d.c tour1d_pp.c tour1d_pp_ui.c \
 tour2d_ui.c tour2d.c tour.c pp_ui.c \
 tourcorr_ui.c tourcorr.c ppcorr_ui.c \
 brush_ui.c brush.c brush_init.c brush_bins.c brush_api.c color_ui.c xlines.c \
 brush_link.c \
 exclusion_ui.c exclusion.c record_id.c \
 scale_ui.c scale_drag.c scale_click.c scale_api.c \
 identify_ui.c identify.c \
 edges.c lineedit_ui.c lineedit.c \
 movepts_ui.c movepts.c \
 parcoords_ui.c parcoords.c \
 scatmat_ui.c scatmat.c \
 varpanel_ui.c noop-checkbutton.c vartable_ui.c vartable.c varchange.c \
 varcircles.c \
 transform_ui.c transform.c sphere_ui.c sphere.c svd.c \
 subset_ui.c subset.c jitter_ui.c jitter.c smooth_ui.c \
 impute_ui.c impute.c \
 display_tree.c \
 ggobi-API.c \
 timeplot.c time_ui.c \
\
 eispack.c \
 gtkextruler.c gtkexthruler.c gtkextvruler.c \
 mt19937-1.c cokus.c \
 fileio.c \
 print.c \
 plugin.c

OB=ggobi.o datad.o make_ggobi.o color.o main_ui.o splash.o cpanel.o \
 menus.o \
 utils.o utils_ui.o utils_gdk.o array.o vector.o \
 read_array.o read_data.o io.o writedata_ui.o writedata.o write_svg.o \
 limits.o pipeline.o missing.o \
 scatterplot.o scatterplot_ui.o \
 splot.o sp_plot.o win32_draw.o \
 display.o display_ui.o \
 p1d_ui.o p1d.o ash1d.o texture.o \
 xyplot_ui.o xyplot.o \
 rotate_ui.o \
 tour2d_ui.o tour2d.o tour.o pp_ui.o tour1d_ui.o tour1d.o tour1d_pp.o tour1d_pp_ui.o \
 tourcorr_ui.o tourcorr.o ppcorr_ui.o \
 brush_ui.o brush.o brush_init.o brush_bins.o brush_api.o color_ui.o xlines.o \
 brush_link.o \
 exclusion_ui.o exclusion.o record_id.o \
 scale_ui.o scale_drag.o scale_click.o scale_api.o \
 identify_ui.o identify.o \
 edges.o lineedit_ui.o lineedit.o \
 movepts_ui.o movepts.o \
 parcoords_ui.o parcoords.o \
 scatmat_ui.o scatmat.o \
 varpanel_ui.o noop-checkbutton.o vartable_ui.o vartable.o varchange.o \
 varcircles.o \
 transform_ui.o transform.o sphere_ui.o sphere.o svd.o \
 subset_ui.o subset.o jitter_ui.o jitter.o smooth_ui.o \
 impute_ui.o impute.o \
 display_tree.o \
 ggobi-API.o \
 timeplot.o time_ui.o \
 eispack.o \
 gtkextruler.o gtkexthruler.o gtkextvruler.o \
 fileio.o \
 print.o  \
 plugin.o


ifdef USE_XML
 XML_SRC= read_xml.c write_xml.c  read_init.c write_state.c
 XML_OB= read_xml.o write_xml.o read_init.o write_state.o

 XML_FLAGS+= -DSUPPORT_PLUGINS=1 -DSUPPORT_INIT_FILES=1
 CFLAGS+= $(XML_INC_DIRS:%=-I%) -DUSE_XML=$(USE_XML) $(XML_FLAGS)

 SRC+=$(XML_SRC)
 OB+= $(XML_OB)

ifdef DL_RESOLVE_FLAG
 DL_RESOLVE_PATH+=$(XML_LIB_DIRS:%=$(DL_RESOLVE_FLAG) %)
endif
ifeq ($(USE_XML),2)
 XML_LIB_NO=2
endif
 XML_LIBS=$(XML_LIB_DIRS:%=-L%) -lxml$(XML_LIB_NO) -lz 

main_ui.o: write_xml.h
read_xml.o: read_xml.h
endif

ifdef USE_MYSQL
 CFLAGS+= $(MYSQL_INCLUDE_DIRS:%=-I%) -DUSE_MYSQL=1 -Wall -I$(PROPERTIES_INCLUDE_DIR) 
 MYSQL_LIBS=-lProps -lmysqlclient $(MYSQL_LIB_DIRS:%=-L%) $(MYSQL_LIB_DIRS:%=$(DL_RESOLVE_FLAG) %) $(PROPERTIES_LIB_DIR:%=$(DL_RESOLVE_FLAG) %) $(PROPERTIES_LIB_DIR:%=-L%)
 SRC+=read_mysql.c
 OB+=read_mysql.o
  LD=$(CXX)
endif

OB+=mt19937-1.o cokus.o  

ggobi: $(OB) $(EXTRA_OB)
	$(LD) $(OB) $(EXTRA_OB) $(LDFLAGS) -o ggobi $(XML_LIBS) $(MYSQL_LIBS)  $(EXTRA_LIBS) `gtk-config --cflags --libs`  $(DL_RESOLVE_PATH)


pure: ggobi.o $(OB)
	purify -cache-dir=/tmp  -always-use-cache-dir=yes \
	$(LD) $(OB) $(LDFLAGS) -o ggobi $(XML_LIBS) $(MYSQL_LIBS) `gtk-config --cflags --libs`  $(DL_RESOLVE_PATH)


%.sched: %.c
	$(CC) -dS -c $(CFLAGS) -I. `gtk-config --cflags` $*.c

ggobi.sched: $(OB)
	$(CC) -S $(OB) $(LDFLAGS)  $(XML_LIB_DIRS:%=-L%) $(XML_LIBS) `gtk-config --cflags --libs`

mt19937-1.o: mt19937-1.c
	$(CC) $(CFLAGS) `gtk-config --cflags` -c $<

efence: $(OB)
	MALLOC_CHECK_=2
	$(CC) $(OB) $(LDFLAGS) $(EFENCE_LIBS) $(XML_LIB_DIRS:%=-L%) $(XML_LIBS) `gtk-config --cflags --libs`

lib: libggobi.so
libggobi.so: $(OB)
	$(CC) -g $(SHARED_LD_FLAGS) -o $@ $(OB) $(XML_LIB_DIRS:%=-L%) $(XML_LIBS) `gtk-config --libs` $(DL_RESOLVE_PATH)


ggobi-API.o ggobi.o: config.h
config.h: developerConfigure
	./developerConfigure

developerConfigure: developerConfigure.in
	autoconf $< > $@
	chmod +x $@


clean: 
	rm -f *.o ggobi libggobi.so

.c.o:
	$(CC) -c -I. $(CFLAGS) `gtk-config --cflags`  $*.c

# A version that compiles all C code (except for mt19937-1.c) as 
# C++.
#	$(CXX) -c $(CXXFLAGS) -I. `gtk-config --cflags` $*.c


ifdef DEPENDS_FLAG
depends: $(SRC)
	$(CC) $(DEPENDS_FLAG) $(CFLAGS) -I. `gtk-config --cflags` $(SRC) > $@	

include depends

endif


# If USE_XML, may need to add XML_LIB_DIRS and XML_INC_DIRS
local.config:
	@echo "# Whether to enable support for reading XML data" > $@
	@echo "# USE_XML=1" >> $@
	@echo "# XML_INC_DIRS=" >> $@
	@echo "# XML_LIB_DIRS=" >> $@
	@echo "# Location of dmalloc" >> $@
	@echo "# DM=1" >> $@


ifdef USE_XML
xmlConvert: xmlConvert.o libggobi.so
	$(CC) -g -o $@ xmlConvert.o $(XML_LIBS) $(XML_LIB_DIRS:%=-L%) -L. -lggobi $(DL_RESOLVE_PATH) -lgtk

make_ggobi.o: read_xml.h
endif


ifdef USE_MYSQL
read_mysql.o: read_mysql.c read_mysql.h
	$(CXX) `gtk-config --cflags` $(CFLAGS) -c $<

sql: read_mysql.o
	$(CC) -o $@ read_mysql.o $(MYSQL_LIBS)

sqldep:
	$(CC) -M $(CFLAGS) read_mysql.c

make_ggobi.o: read_mysql.h
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


print.o: print.c print.h

%.o: %.d

%.d: %.c
	$(CC) -M $(CFLAGS) -I. `gtk-config --cflags` $< > $@

# DO NOT DELETE
