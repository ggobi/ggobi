#ifndef SCATMAT_CLASS_H
#define SCATMAT_CLASS_H

#include "ggobi.h"

#define GTK_TYPE_GGOBI_SCATMAT_SPLOT          (gtk_ggobi_scatmat_splot_get_type())
#define GTK_GGOBI_SCATMAT_SPLOT(obj)	        (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_SCATMAT_SPLOT, scatmatSPlotd))
#define GTK_GGOBI_SCATMAT_SPLOT_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_SCATMAT_SPLOT, GtkGGobiScatmatSPlotClass))
#define GTK_IS_GGOBI_SCATMAT_SPLOT(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_SCATMAT_SPLOT))
#define GTK_IS_GGOBI_SCATMAT_SPLOT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_SCATMAT_SPLOT))

GtkType gtk_ggobi_scatmat_splot_get_type(void);


typedef struct 
{
    GtkGGobiExtendedSPlotClass parent_class;

} GtkGGobiScatmatSPlotClass;

typedef  struct {

    extendedSPlotd extendedSPlot;

} scatmatSPlotd;




#define GTK_TYPE_GGOBI_SCATMAT_DISPLAY	 (gtk_ggobi_scatmat_display_get_type ())
#define GTK_GGOBI_SCATMAT_DISPLAY(obj)	 (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_SCATMAT_DISPLAY, scatmatDisplayd))
#define GTK_GGOBI_SCATMAT_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_SCATMAT_DISPLAY, GtkGGobiScatmatDisplayClass))
#define GTK_IS_GGOBI_SCATMAT_DISPLAY(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_SCATMAT_DISPLAY))
#define GTK_IS_GGOBI_SCATMAT_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_SCATMAT_DISPLAY))

GtkType gtk_ggobi_scatmat_display_get_type();
displayd *gtk_ggobi_scatmat_display_new(gint type, gboolean missing_p, datad *d, ggobid *gg);

typedef struct 
{
    GtkGGobiExtendedDisplayClass parent_class;


} GtkGGobiScatmatDisplayClass;


typedef struct {

  extendedDisplayd extendedDpy;
 
} scatmatDisplayd;


void scatmatDisplayClassInit(GtkGGobiScatmatDisplayClass *klass);
void scatmatSPlotClassInit(GtkGGobiScatmatSPlotClass *klass) ;


#endif

