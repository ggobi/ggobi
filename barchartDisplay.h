#ifndef GGOBI_BARCHART_DISPLAY_H
#define GGOBI_BARCHART_DISPLAY_H

/**
 This defines a new class of display (GtkGGobiTimeSeriesDisplay)
 which is the top-level container for multiple time series plots.
 This extends the windowed display class (GtkGGobiWindowDisplay).
*/

/*
 Is it necessary/useful to sub-class these
*/

#define GTK_TYPE_GGOBI_BARCHART_DISPLAY	 (gtk_ggobi_barchart_display_get_type ())
#define GTK_GGOBI_BARCHART_DISPLAY(obj)	 (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_BARCHART_DISPLAY, barchartDisplayd))
#define GTK_GGOBI_BARCHART_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_BARCHART_DISPLAY, GtkGGobiBarChartClass))
#define GTK_IS_GGOBI_BARCHART_DISPLAY(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_BARCHART_DISPLAY))
#define GTK_IS_GGOBI_BARCHART_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_BARCHART_DISPLAY))

GtkType gtk_ggobi_barchart_display_get_type();
displayd *gtk_ggobi_barchart_display_new(gint type, gboolean missing_p,
                                         datad * d, ggobid * gg);

typedef struct {
  GtkGGobiExtendedDisplayClass parent_class;

} GtkGGobiBarChartDisplayClass;


typedef struct {

  extendedDisplayd extendedDpy;

} barchartDisplayd;



#define GTK_TYPE_GGOBI_BARCHART_SPLOT           (gtk_ggobi_barchart_splot_get_type())
#define GTK_GGOBI_BARCHART_SPLOT(obj)	        (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_BARCHART_SPLOT, barchartSPlotd))
#define GTK_GGOBI_BARCHART_SPLOT_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_BARCHART_SPLOT, GtkGGobiBarChartSPlotClass))
#define GTK_IS_GGOBI_BARCHART_SPLOT(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_BARCHART_SPLOT))
#define GTK_IS_GGOBI_BARCHART_SPLOT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_BARCHART_SPLOT))

GtkType gtk_ggobi_barchart_splot_get_type(void);

typedef struct {
  GtkGGobiExtendedSPlotClass extendedSPlotClass;

} GtkGGobiBarChartSPlotClass;

typedef struct {

  extendedSPlotd extendedSPlot;

  barchartd *bar;
} barchartSPlotd;




extern void barchart_display_menus_make(displayd * display,
                                        GtkAccelGroup *, GtkSignalFunc,
                                        ggobid *);
extern splotd *gtk_barchart_splot_new(displayd * dpy, gint width,
                                      gint height, ggobid * gg);

/* Make these available to ggobiClass.c. 
  (Could put the _get_type routines in the barchartClass.h file.) */

void barchartSPlotClassInit(GtkGGobiBarChartSPlotClass * klass);
void barchartDisplayClassInit(GtkGGobiBarChartDisplayClass * klass);
#endif
