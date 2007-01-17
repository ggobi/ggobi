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
 This defines a new class of display (GGobiTimeSeriesDisplay)
 which is the top-level container for multiple time series plots.
 This extends the windowed display class (GGobiWindowDisplay).
*/



#define GGOBI_TYPE_TIME_SERIES_SPLOT           (ggobi_time_series_splot_get_type())
#define GGOBI_TIME_SERIES_SPLOT(obj)	        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_TIME_SERIES_SPLOT, timeSeriesSPlotd))
#define GGOBI_TIME_SERIES_SPLOT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_TIME_SERIES_SPLOT, GGobiTimeSeriesSPlotClass))
#define GGOBI_IS_TIME_SERIES_SPLOT(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_TIME_SERIES_SPLOT))
#define GGOBI_IS_TIME_SERIES_SPLOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_TIME_SERIES_SPLOT))
#define GGOBI_TIME_SERIES_SPLOT_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_TIME_SERIES_SPLOT, GGobiTimeSeriesSPlotClass))

GType ggobi_time_series_splot_get_type(void);

typedef struct 
{
    GGobiExtendedSPlotClass extendedSPlotClass;

} GGobiTimeSeriesSPlotClass;

typedef  struct {

    extendedSPlotd extendedSPlot;

} timeSeriesSPlotd;




#define GGOBI_TYPE_TIME_SERIES_DISPLAY	 (ggobi_time_series_display_get_type ())
#define GGOBI_TIME_SERIES_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_TIME_SERIES_DISPLAY, timeSeriesDisplayd))
#define GGOBI_TIME_SERIES_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_TIME_SERIES_DISPLAY, GGobiTimeSeriesDisplayClass))
#define GGOBI_IS_TIME_SERIES_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_TIME_SERIES_DISPLAY))
#define GGOBI_IS_TIME_SERIES_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_TIME_SERIES_DISPLAY))
#define GGOBI_TIME_SERIES_DISPLAY_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_TIME_SERIES_DISPLAY, GGobiTimeSeriesDisplayClass))

GType ggobi_time_series_display_get_type();
displayd *ggobi_time_series_display_new(gint type, gboolean missing_p, GGobiStage *d, GGobiSession *gg);

typedef struct 
{
    GGobiExtendedDisplayClass parent_class;


} GGobiTimeSeriesDisplayClass;


typedef struct _timeSeriesDisplayd {

  extendedDisplayd extendedDpy;
 
} timeSeriesDisplayd;


 /* Making these available to ggobiClass.c */
displayd *timeSeriesDisplayCreate(gboolean missing_p, splotd *sp, GGobiStage *d, GGobiSession *gg);
gint tsplotIsVarPlotted(displayd *display, GSList *cols, GGobiStage *d);
gboolean tsplotCPanelSet(displayd *dpy, cpaneld *cpanel, GGobiSession *gg);
void tsplotDisplaySet(displayd *dpy, GGobiSession *gg);
void tsplotVarpanelRefresh(displayd *display, splotd *sp, GGobiStage *d);
gboolean tsplotHandlesProjection(displayd *dpy, ProjectionMode mode);
gboolean tsplotHandlesInteraction(displayd *, InteractionMode);

#ifdef STORE_SESSION_ENABLED
void add_xml_tsplot_variables(xmlNodePtr node, GList *plots, displayd *dpy);
#endif
void tsplotVarpanelTooltipsSet(displayd *dpy, GGobiSession *gg, GtkWidget *wx, GtkWidget *wy, GtkWidget *wz, GtkWidget *label);
gint tsplotPlottedColsGet(displayd *display, gint *cols, GGobiStage *d, GGobiSession *gg);

//GtkWidget *tsplotMenusMake(displayd *dpy, GGobiSession *gg);

GtkWidget *tsplotCPanelWidget(displayd *dpy, gchar **modeName, GGobiSession *gg);

gboolean tsplotEventHandlersToggle(displayd *dpy, splotd *sp, gboolean state, ProjectionMode, InteractionMode imode);
gboolean tsplotKeyEventHandled(GtkWidget *, displayd *, splotd *sp, GdkEventKey *, GGobiSession *);
gchar *tsplot_tree_label(splotd *sp, GGobiStage *d, GGobiSession *gg);

GdkSegment * tsplotAllocWhiskers(displayd *dpy, splotd *sp, gint nrows, GGobiStage *d);
void tsplotAddPlotLabels(displayd *display, splotd *sp, GdkDrawable *drawable, GGobiStage *d, GGobiSession *gg);



splotd *ggobi_time_series_splot_new(displayd *dpy, GGobiSession *gg);

#endif

