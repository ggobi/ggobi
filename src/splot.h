/*-- splot.h: the variables required for each single plot --*/
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
*/

#ifndef SPLOT_H
#define SPLOT_H

#include "defines.h"

typedef struct _displayd displayd;

#define GGOBI_TYPE_SPLOT     (ggobi_splot_get_type ())
#define GGOBI_SPLOT(obj)	 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_SPLOT, splotd))
#define GGOBI_SPLOT_CLASS(klass)	 (G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_SPLOT, GGobiSPlotClass))
#define GGOBI_IS_SPLOT(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_SPLOT))
#define GGOBI_IS_SPLOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_SPLOT))
#define GGOBI_SPLOT_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_SPLOT, GGobiSPlotClass))


GType ggobi_splot_get_type(void);

typedef struct 
{
    GtkDrawingAreaClass parent_class;

    RedrawStyle redraw; /* used by barchart to indicate it needs to do a full redraw from set_color_id. */

} GGobiSPlotClass;


typedef struct 
{

 GtkDrawingArea canvas;

 displayd *displayptr;  /* a pointer to the enclosing display */

 GtkWidget *da;                 /* drawing_area */
 GdkPixmap *pixmap0, *pixmap1;  /* 2-stage drawing */

 GdkCursor *cursor;
 gint jcursor;

 RedrawStyle redraw_style;

 /* Drawing area dimensions */
 icoords max;

 /*
  * line segments in scatterplot and scatmat
 */
 GdkSegment *arrowheads;
 GdkSegment *edges;

 /*
  * line segments in parallel coordinates plot
 */
 GdkSegment *whiskers;

 gcoords *planar;
 icoords *screen;

 /*
  * shift and scale
 */
 fcoords scale, tour_scale;
 gcoords iscale;
 gcoords pmid;	  /*-- center of the screen in planar/world coords --*/

/*
 * button and key event information
*/
 gint motion_id, press_id, release_id, key_press_id, scroll_id;
 icoords mousedownpos, mousepos, mousepos_o;

 brush_coords brush_pos, brush_pos_o;  

/*
 * plot1d  (used in parcoords as well as scatterplot)
*/
 gint p1dvar;
 struct _p1d {
   vector_f spread_data; /* the spreading data */
   lims lim;      /* limits of the spreading data */
   gfloat mean;
   /*-- line segments in ASHes --*/
   icoords ash_baseline;  /*-- for drawing segments from point to baseline --*/
 } p1d;
 

/*
 * tour1d (variables used to record the state of the 1D tour for this plot)
 */
 struct _tour1d {
   gfloat mincnt;
   gfloat maxcnt;
   gfloat minscreenx;
   gfloat maxscreenx;
   gboolean initmax;
   icoords ash_baseline;  /*-- for drawing segments from point to baseline --*/
 } tour1d;

/*
 * tour2d: rescaling so that points don't go outside the planar space
 */
 struct _tour2d {
   greal maxscreen;
   gboolean initmax;
 } tour2d;

/*
 * tour2d: rescaling so that points don't go outside the planar space
 */
 struct _tourcorr {
   greal maxscreen;
   gboolean initmax;
 } tourcorr;

/*
 * xyplot
*/
 icoords xyvars;

/*-- rotation implemented as a limited tour2d --*/
 struct _tour2d3 {
   greal maxscreen;
   gboolean initmax;
 } tour2d3;

#ifdef WIN32
 struct _win32 {
   gint       npoints;
   GdkPoint   *points;
   GdkSegment *segs;
   GdkSegment *whisker_segs;
   GdkSegment *ash_segs;
   GdkRectangle      *open_rects;
   GdkRectangle      *filled_rects;
   GdkRectangle       *open_arcs;
   GdkRectangle       *filled_arcs;
 } win32;
#endif

} splotd;


#define GGOBI_TYPE_EXTENDED_SPLOT           (ggobi_extended_splot_get_type())
#define GGOBI_EXTENDED_SPLOT(obj)	        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_EXTENDED_SPLOT, extendedSPlotd))
#define GGOBI_EXTENDED_SPLOT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_EXTENDED_SPLOT, GGobiExtendedSPlotClass))
#define GGOBI_IS_EXTENDED_SPLOT(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_EXTENDED_SPLOT))
#define GGOBI_IS_EXTENDED_SPLOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_EXTENDED_SPLOT))
#define GGOBI_EXTENDED_SPLOT_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_EXTENDED_SPLOT, GGobiExtendedSPlotClass))

GType ggobi_extended_splot_get_type(void);

#include "ggobi-data.h"

typedef struct 
{
  GGobiSPlotClass splot;

  gchar *(*tree_label)(splotd *sp, GGobiStage *d, ggobid *gg);

  gboolean (*identify_notify)(icoords, splotd *, GGobiStage *, ggobid *);

  void (*add_plot_labels)(splotd *, GdkDrawable *, ggobid *gg);
  void (*add_markup_cues)(splotd *, GdkDrawable *, ggobid *);
  void (*add_scaling_cues)(splotd *, GdkDrawable *, ggobid *);
  void (*add_identify_cues)(gboolean, gint k, splotd *, GdkDrawable *, ggobid *);
  void (*add_identify_edge_cues)(gint k, splotd *, GdkDrawable *, gboolean, ggobid *);

  gboolean (*redraw)(splotd *, GGobiStage *, ggobid *, gboolean binned);

  void (*screen_to_tform)(cpaneld *, splotd *, icoords *, fcoords *, ggobid *);
  void (*world_to_plane)(splotd *, GGobiStage *, ggobid *);

  /** Convenience to be called within the standard loop */
  void (*sub_plane_to_screen)(splotd *sp, displayd *dpy, GGobiStage *d, ggobid *gg);
      /** Allows the class to take over the entire plane_to_screen.
          Handling each row can be done using a method for sub_plane_to_screen. */
  void (*plane_to_screen)(splotd *, GGobiStage *, ggobid *);

  gint (*active_paint_points)(splotd *, GGobiStage *, ggobid *);


  GdkSegment *(*alloc_whiskers)(GdkSegment *, splotd *sp, gint nrows, GGobiStage *d);

  /** called from splot_plot_edge */
  gboolean (*draw_edge_p)(splotd *sp, gint m, GGobiStage *d, GGobiStage *e, ggobid *gg);
  /** called from splot_plot_case. Should probably be the same as
      draw_edge_p but doesn't take the edge argument! Could drop the
      first datad in splot_plot_edge and just hand it the one dataset. */
  gboolean (*draw_case_p)(splotd *sp, gint m, GGobiStage *d, ggobid *gg);

  void (*within_draw_to_binned)(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc);
  void (*within_draw_to_unbinned)(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc);

  void (*splot_assign_points_to_bins)(GGobiStage *, splotd *, ggobid *);

  gint (*plotted_vars_get)(splotd *sp, gint *vars, GGobiStage *d);


  splotd * (*createWithVars)(displayd *dpy, gint *vars, gint nvars, ggobid *gg);

} GGobiExtendedSPlotClass;

typedef struct 
{

   splotd splot; 

} extendedSPlotd;



void splot_init(splotd *sp, displayd *display, struct _ggobid *gg);

/* shared by barchart and parcoords. */
gint splot1DVariablesGet(splotd *sp, gint *cols, GGobiStage *d);

#endif
