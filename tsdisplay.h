/* tsdisplay.h */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site,
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
 *
 * Contributing author of time series code:  Nicholas Lewin-Koh
*/


#ifndef GGOBI_TSDISPLAY_H
#define GGOBI_TSDISPLAY_H

/**
 This defines a new class of display (GtkGGobiTimeSeriesDisplay)
 which is the top-level container for multiple time series plots.
 This extends the windowed display class (GtkGGobiWindowDisplay).
*/



#define GTK_TYPE_GGOBI_TS_SPLOT           (gtk_ggobi_time_series_splot_get_type())
#define GTK_GGOBI_TS_SPLOT(obj)	        (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_TS_SPLOT, timeSeriesSPlotd))
#define GTK_GGOBI_TS_SPLOT_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_TS_SPLOT, GtkGGobiTimeSeriesSPlotClass))
#define GTK_IS_GGOBI_TS_SPLOT(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_TS_SPLOT))
#define GTK_IS_GGOBI_TS_SPLOT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_TS_SPLOT))

GtkType gtk_ggobi_time_series_splot_get_type(void);

typedef struct 
{
    GtkGGobiExtendedSPlotClass extendedSPlotClass;

} GtkGGobiTimeSeriesSPlotClass;

typedef  struct {

    extendedSPlotd extendedSPlot;

} timeSeriesSPlotd;




#define GTK_TYPE_GGOBI_TIME_SERIES_DISPLAY	 (gtk_ggobi_time_series_display_get_type ())
#define GTK_GGOBI_TIME_SERIES_DISPLAY(obj)	 (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_TIME_SERIES_DISPLAY, timeSeriesDisplayd))
#define GTK_GGOBI_TIME_SERIES_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_TIME_SERIES_DISPLAY, GtkGGobiTimeSeriesDisplayClass))
#define GTK_IS_GGOBI_TIME_SERIES_DISPLAY(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_TIME_SERIES_DISPLAY))
#define GTK_IS_GGOBI_TIME_SERIES_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_TIME_SERIES_DISPLAY))

GtkType gtk_ggobi_time_series_display_get_type();
displayd *gtk_ggobi_time_series_display_new(gint type, gboolean missing_p, datad *d, ggobid *gg);

typedef struct 
{
    GtkGGobiExtendedDisplayClass parent_class;


} GtkGGobiTimeSeriesDisplayClass;


typedef struct _timeSeriesDisplayd {

  extendedDisplayd extendedDpy;
 
} timeSeriesDisplayd;


 /* Making these available to ggobiClass.c */
displayd *timeSeriesDisplayCreate(gboolean missing_p, splotd *sp, datad *d, ggobid *gg);
gint tsplotIsVarPlotted(displayd *display, gint *cols, gint ncols, datad *d);
gboolean tsplotCPanelSet(displayd *dpy, cpaneld *cpanel, ggobid *gg);
void tsplotDisplaySet(displayd *dpy, ggobid *gg);
void tsplotVarpanelRefresh(displayd *display, splotd *sp, datad *d);
gboolean tsplotHandlesAction(displayd *dpy, PipelineMode mode);
void add_xml_tsplot_variables(xmlNodePtr node, GList *plots, displayd *dpy);
void tsplotVarpanelTooltipsSet(displayd *dpy, ggobid *gg, GtkWidget *wx, GtkWidget *wy, GtkWidget *wz, GtkWidget *label);
gint tsplotPlottedColsGet(displayd *display, gint *cols, datad *d, ggobid *gg);

GtkWidget *tsplotMenusMake(displayd *dpy, PipelineMode viewMode, ggobid *gg);

GtkWidget *tsplotCPanelWidget(displayd *dpy, gint viewmode, gchar **modeName, ggobid *gg);
gboolean tsplotEventHandlersToggle(displayd *dpy, splotd *sp, gboolean state, gint viewMode);
gint tsplotSPlotKeyEventHandler(displayd *dpy, splotd *sp, gint keval);
gchar *tsplot_tree_label(splotd *sp, datad *d, ggobid *gg);

GdkSegment * tsplotAllocWhiskers(displayd *dpy, splotd *sp, gint nrows, datad *d);
void tsplotAddPlotLabels(displayd *display, splotd *sp, GdkDrawable *drawable, datad *d, ggobid *gg);


splotd *gtk_time_series_splot_new(displayd *dpy, gint width, gint height, ggobid *gg);

#endif

