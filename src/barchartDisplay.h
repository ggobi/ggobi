/* barchartDisplay.h */
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
 * Contributing author of barchart and histogram code:  Heike Hofmann
*/

 
#ifndef GGOBI_BARCHART_DISPLAY_H
#define GGOBI_BARCHART_DISPLAY_H

/**
 This defines a new class of display (GGobiBarchartDisplay)
 which is the top-level container for barchart plots.
 This extends the windowed display class (GGobiWindowDisplay).
*/

/*
 Is it necessary/useful to sub-class these
*/

#define GGOBI_TYPE_BARCHART_DISPLAY	 (ggobi_barchart_display_get_type ())
#define GGOBI_BARCHART_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_BARCHART_DISPLAY, barchartDisplayd))
#define GGOBI_BARCHART_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_BARCHART_DISPLAY, GGobiBarChartClass))
#define GGOBI_IS_BARCHART_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_BARCHART_DISPLAY))
#define GGOBI_IS_BARCHART_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_BARCHART_DISPLAY))
#define GGOBI_BARCHART_DISPLAY_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_BARCHART_DISPLAY, GGobiBarChartClass))

GType ggobi_barchart_display_get_type();
displayd *ggobi_barchart_display_new(gint type, gboolean missing_p,
                                         GGobiStage * d, GGobiSession * gg);

typedef struct {
  GGobiExtendedDisplayClass parent_class;

} GGobiBarChartDisplayClass;


typedef struct {

  extendedDisplayd extendedDpy;

} barchartDisplayd;



#define GGOBI_TYPE_BARCHART_SPLOT           (ggobi_barchart_splot_get_type())
#define GGOBI_BARCHART_SPLOT(obj)	        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_BARCHART_SPLOT, barchartSPlotd))
#define GGOBI_BARCHART_SPLOT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_BARCHART_SPLOT, GGobiBarChartSPlotClass))
#define GGOBI_IS_BARCHART_SPLOT(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_BARCHART_SPLOT))
#define GGOBI_IS_BARCHART_SPLOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_BARCHART_SPLOT))
#define GGOBI_BARCHART_SPLOT_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_BARCHART_SPLOT, GGobiBarChartSPlotClass))

GType ggobi_barchart_splot_get_type(void);

typedef struct {
  GGobiExtendedSPlotClass extendedSPlotClass;

} GGobiBarChartSPlotClass;

typedef struct {

  extendedSPlotd extendedSPlot;

  barchartd *bar;
} barchartSPlotd;




extern void barchart_display_menus_make(displayd * display,
                                        GtkAccelGroup *, GtkSignalFunc,
                                        GGobiSession *);
extern splotd *ggobi_barchart_splot_new(displayd * dpy, GGobiSession * gg);

/* Make these available to ggobiClass.c. 
  (Could put the _get_type routines in the barchartClass.h file.) */

void barchartSPlotClassInit(GGobiBarChartSPlotClass * klass);
void barchartDisplayClassInit(GGobiBarChartDisplayClass * klass);
#endif
