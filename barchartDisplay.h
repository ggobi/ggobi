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
displayd *gtk_ggobi_barchart_display_new(gint type, gboolean missing_p, datad *d, ggobid *gg);

typedef struct 
{
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

typedef struct 
{
    GtkGGobiExtendedSPlotClass extendedSPlotClass;

} GtkGGobiBarChartSPlotClass;

typedef  struct {

    extendedSPlotd extendedSPlot;

    barchartd *bar;
} barchartSPlotd;




extern void barchart_display_menus_make (displayd *display, GtkAccelGroup *, GtkSignalFunc, ggobid *);
extern splotd *gtk_barchart_splot_new(displayd *dpy, gint width, gint height, ggobid *gg);


  /* Methods for barchart splot. */
gchar *barchart_tree_label(splotd *sp, datad *d, ggobid *gg);


 /* Making these available to ggobiClass.c */
gboolean barchartVarSel(displayd *display, splotd *sp, gint jvar, gint btn, cpaneld *cpanel, ggobid *gg);
gint barchartVarIsPlotted(displayd *dpy, gint *cols, gint ncols, datad *d);
gboolean barchartCPanelSet(displayd *dpy, cpaneld *cpanel, ggobid *gg);
void barchartDisplaySet(displayd *dpy, ggobid *gg);
void barchartDestroy(barchartSPlotd *sp);
void barchartPlaneToScreen(splotd *sp, datad *d, ggobid *gg);

gboolean barchart_build_symbol_vectors (datad *d, ggobid *gg);
void barchartVarpanelRefresh(displayd *display, splotd *sp, datad *d);
gboolean barchartHandlesAction(displayd *dpy, PipelineMode mode);
void barchartVarpanelTooltipsSet(displayd *dpy, ggobid *gg, GtkWidget *wx, GtkWidget *wy, GtkWidget *label);
gint barchartPlottedColsGet(displayd *display, gint *cols, datad *d, ggobid *gg);
GtkWidget *barchartCPanelWidget(displayd *dpy, gint viewmode, gchar **modeName, ggobid *gg);
GtkWidget *barchartMenusMake(displayd *dpy, PipelineMode viewMode, ggobid *gg);
gboolean barchartEventHandlersToggle(displayd *dpy, splotd *sp, gboolean state, gint viewMode);
gint  barchartSPlotKeyEventHandler(displayd *dpy, splotd *sp, gint keyval);
#endif

