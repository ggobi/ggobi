EXTRAS = -Wpointer-arith -Wcast-qual -Wcast-align

CC = gcc
CFLAGS= -g -ansi -Wall -fpic
SHARED_LD_FLAGS= -shared

#DEPENDS_FLAG=-MM

SRC=ggobi.c make_ggobi.c color.c main_ui.c cpanel.c utils.c array.c vector.c \
 read_array.c read_data.c \
 pipeline.c missing.c \
 scatterplot.c scatterplot_ui.c \
 splot.c sp_plot.c win32_draw.c \
 display.c \
 p1d_ui.c p1d.c ash1d.c texture.c \
 xyplot_ui.c xyplot.c \
 rotate_ui.c \
 tour2d_ui.c tour.c pp_ui.c \
 ctour_ui.c cpp_ui.c \
 brush_ui.c brush.c brush_init.c brush_bins.c brush_api.c color_ui.c \
 exclusion_ui.c exclusion.c \
 scale_ui.c scale_drag.c scale_click.c \
 identify_ui.c identify.c \
 lineedit_ui.c lineedit.c \
 movepts_ui.c \
 parcoords_ui.c parcoords.c \
 scatmat_ui.c scatmat.c \
 varpanel_ui.c vartable_ui.c vardata.c \
 transform_ui.c transform.c sphere_ui.c sphere.c svd.c \
 subset_ui.c subset.c jitter_ui.c jitter.c smooth_ui.c \
 gtkextruler.c gtkexthruler.c gtkextvruler.c \
 display_tree.c \
 ggobi-API.c

OB=ggobi.o make_ggobi.o color.o main_ui.o cpanel.o array.o vector.o \
 utils_ui.o utils.o utils_gdk.o \
 read_array.o read_data.o \
 pipeline.o missing.o \
 scatterplot.o scatterplot_ui.o \
 splot.o sp_plot.o win32_draw.o \
 display.o \
 p1d_ui.o p1d.o ash1d.o texture.o \
 xyplot_ui.o xyplot.o \
 rotate_ui.o \
 tour2d_ui.o tour.o pp_ui.o \
 ctour_ui.o cpp_ui.o \
 brush_ui.o brush.o brush_init.o brush_bins.o brush_api.o color_ui.o \
 exclusion_ui.o exclusion.o \
 scale_ui.o scale_drag.o scale_click.o \
 identify_ui.o identify.o \
 lineedit_ui.o lineedit.o \
 movepts_ui.o \
 parcoords_ui.o parcoords.o \
 scatmat_ui.o scatmat.o \
 varpanel_ui.o vartable_ui.o vardata.o \
 transform_ui.o transform.o sphere_ui.o sphere.o svd.o \
 subset_ui.o subset.o jitter_ui.o jitter.o smooth_ui.o \
 gtkextruler.o gtkexthruler.o gtkextvruler.o \
 display_tree.o \
 ggobi-API.o

ggobi: $(OB)
	$(CC) $(OB) -o ggobi `gtk-config --cflags --libs`

pure: ggobi.o $(OB)
	purify -cache-dir=/tmp  -always-use-cache-dir=yes \
	${CC} -o ggobi $(OB) `gtk-config --cflags --libs`

# No gthread here in the call to gtk-config.
# R is not compiled for threads.
libGGobi.so: $(OB)
	$(CC) -g $(SHARED_LD_FLAGS) -o $@ $(OB) `gtk-config --libs`

clean: 
	rm -f *.o ggobi libGGobi.so

.c.o:
	$(CC) -c $(CFLAGS) -I. `gtk-config --cflags` $*.c



# depends: $(SRC)
# 	$(CC) $(DEPENDS_FLAG) $(CFLAGS) -I. `gtk-config --cflags` $(SRC) > $@	


# include .depends
# DO NOT DELETE
