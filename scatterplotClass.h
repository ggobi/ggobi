#ifndef SCATTERPLOT_CLASS_H
#define SCATTERPLOT_CLASS_H


#include "ggobi.h"


#define GTK_TYPE_GGOBI_SCATTER_SPLOT          (gtk_ggobi_scatter_splot_get_type())
#define GTK_GGOBI_SCATTER_SPLOT(obj)	        (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_SCATTER_SPLOT, scatterSPlotd))
#define GTK_GGOBI_SCATTER_SPLOT_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_SCATTER_SPLOT, GtkGGobiScatterSPlotClass))
#define GTK_IS_GGOBI_SCATTER_SPLOT(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_SCATTER_SPLOT))
#define GTK_IS_GGOBI_SCATTER_SPLOT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_SCATTER_SPLOT))

GtkType gtk_ggobi_scatter_splot_get_type(void);


typedef struct 
{
    GtkGGobiExtendedSPlotClass parent_class;

} GtkGGobiScatterSPlotClass;

typedef  struct {

    extendedSPlotd extendedSPlot;

} scatterSPlotd;



#define GTK_TYPE_GGOBI_SCATTERPLOT_DISPLAY	 (gtk_ggobi_scatterplot_display_get_type ())
#define GTK_GGOBI_SCATTERPLOT_DISPLAY(obj)	 (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_SCATTERPLOT_DISPLAY, scatterplotDisplayd))
#define GTK_GGOBI_SCATTERPLOT_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_SCATTERPLOT_DISPLAY, GtkGGobiScatterplotDisplayClass))
#define GTK_IS_GGOBI_SCATTERPLOT_DISPLAY(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_SCATTERPLOT_DISPLAY))
#define GTK_IS_GGOBI_SCATTERPLOT_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_SCATTERPLOT_DISPLAY))

GtkType gtk_ggobi_scatterplot_display_get_type();
displayd *gtk_ggobi_scatterplot_display_new(gint type, gboolean missing_p, datad *d, ggobid *gg);

typedef struct 
{
    GtkGGobiExtendedDisplayClass parent_class;


} GtkGGobiScatterplotDisplayClass;


typedef struct {

  extendedDisplayd extendedDpy;
 
} scatterplotDisplayd;


void scatterSPlotClassInit(GtkGGobiScatterSPlotClass *klass);
void scatterplotDisplayClassInit(GtkGGobiScatterplotDisplayClass *display);
void scatterplotDisplayInit(scatterplotDisplayd *display);


displayd *createScatterplot(displayd *, gboolean missing_p, splotd *sp, gint numVars, gint *vars, datad *d, ggobid *gg);

#endif

