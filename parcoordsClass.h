#ifndef PARCOORDS_CLASS_H
#define PARCOORDS_CLASS_H

#include "ggobi.h"

#define GTK_TYPE_GGOBI_PARCOORDS_SPLOT          (gtk_ggobi_par_coords_splot_get_type())
#define GTK_GGOBI_PARCOORDS_SPLOT(obj)	        (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_PARCOORDS_SPLOT, parcoordsSPlotd))
#define GTK_GGOBI_PARCOORDS_SPLOT_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_PARCOORDS_SPLOT, GtkGGobiParCoordsSPlotClass))
#define GTK_IS_GGOBI_PARCOORDS_SPLOT(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_PARCOORDS_SPLOT))
#define GTK_IS_GGOBI_PARCOORDS_SPLOT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_PARCOORDS_SPLOT))

GtkType gtk_ggobi_par_coords_splot_get_type(void);

typedef struct 
{
    GtkGGobiExtendedSPlotClass parent_class;

} GtkGGobiParCoordsSPlotClass;

typedef  struct {

    extendedSPlotd extendedSPlot;

} parcoordsSPlotd;



#define GTK_TYPE_GGOBI_PARCOORDS_DISPLAY	 (gtk_ggobi_par_coords_display_get_type ())
#define GTK_GGOBI_PARCOORDS_DISPLAY(obj)	 (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_PARCOORDS_DISPLAY, parcoordsDisplayd))
#define GTK_GGOBI_PARCOORDS_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_PARCOORDS_DISPLAY, GtkGGobiParCoordsDisplayClass))
#define GTK_IS_GGOBI_PARCOORDS_DISPLAY(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_PARCOORDS_DISPLAY))
#define GTK_IS_GGOBI_PARCOORDS_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_PARCOORDS_DISPLAY))

GtkType gtk_ggobi_par_coords_display_get_type();
displayd *gtk_ggobi_par_coords_display_new(gint type, gboolean missing_p, datad *d, ggobid *gg);

typedef struct 
{
    GtkGGobiExtendedDisplayClass parent_class;


} GtkGGobiParCoordsDisplayClass;


typedef struct {

  extendedDisplayd extendedDpy;
 
} parcoordsDisplayd;


void parcoordsDisplayClassInit(GtkGGobiParCoordsDisplayClass *klass);
void parcoordsSPlotClassInit(GtkGGobiParCoordsSPlotClass *klass);
void parcoordsDisplayInit(parcoordsDisplayd *display);
splotd *gtk_parcoords_splot_new(displayd *dpy, gint width, gint height, ggobid *gg);

#endif

