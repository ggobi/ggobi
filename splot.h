/*-- splot.h: the variables required for each single plot --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#ifndef SPLOT_H
#define SPLOT_H

#include "defines.h"

typedef struct _displayd displayd;

#define GTK_TYPE_GGOBI_SPLOT     (gtk_ggobi_splot_get_type ())
#define GTK_GGOBI_SPLOT(obj)	 (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_SPLOT, splotd))
#define GTK_GGOBI_SPLOT_CLASS(klass)	 (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_SPLOT, GtkGGobiSPlotClass))
#define GTK_IS_GGOBI_SPLOT(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_SPLOT))
#define GTK_IS_GGOBI_SPLOT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_SPLOT))

GtkType gtk_ggobi_splot_get_type(void);

typedef struct 
{
    GtkDrawingAreaClass parent_class;

    RedrawStyle redraw; /* used by barchart to indicate it needs to do a full redraw from set_color_id. */

} GtkGGobiSPlotClass;


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

 lcoords *planar;
 icoords *screen;

 /*
  * shift and scale
 */
 fcoords scale, tour_scale;
 lcoords iscale;
 lcoords pmid;	  /*-- center of the screen in planar coords --*/

/*
 * button and key event information
*/
 gint motion_id, press_id, release_id, key_press_id;
 icoords mousepos, mousepos_o;

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
   /*gint keepmin;
   gint keepmax;*/ /* Is this ever used */
   /*gboolean firsttime;*/
   icoords ash_baseline;  /*-- for drawing segments from point to baseline --*/
 } tour1d;

/*
 * xyplot
*/
 icoords xyvars;

#ifdef ROTATION_IMPLEMENTED
/*
 * rotation
*/
 struct {gint x, y, z;} spinvars;
#endif

} splotd;


#define GTK_TYPE_GGOBI_EXTENDED_SPLOT           (gtk_ggobi_extended_splot_get_type())
#define GTK_GGOBI_EXTENDED_SPLOT(obj)	        (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_EXTENDED_SPLOT, extendedSPlotd))
#define GTK_GGOBI_EXTENDED_SPLOT_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_EXTENDED_SPLOT, GtkGGobiExtendedSPlotClass))
#define GTK_IS_GGOBI_EXTENDED_SPLOT(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_EXTENDED_SPLOT))
#define GTK_IS_GGOBI_EXTENDED_SPLOT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_EXTENDED_SPLOT))

GtkType gtk_ggobi_extended_splot_get_type(void);

#include "datad.h"

typedef struct 
{

   GtkGGobiSPlotClass splot;

   gchar *(*tree_label)(splotd *sp, datad *d, ggobid *gg);
 
   gboolean (*identify_notify)(icoords, splotd *, datad *, ggobid *);

   void (*add_plot_labels)(splotd *, GdkDrawable *, ggobid *gg);
   void (*add_markup_cues)(splotd *, GdkDrawable *, ggobid *);
   void (*add_scaling_cues)(splotd *, GdkDrawable *, ggobid *);

   gboolean (*redraw)(splotd *, datad *, ggobid *, gboolean binned);

   void (*world_to_plane)(splotd *, datad *, ggobid *);

	/** Convenience to be called within the standard loop */
   void (*sub_plane_to_screen)(splotd *sp, displayd *dpy, datad *d, ggobid *gg);
        /** Allows the class to take over the entire plane_to_screen.
            Handling each row can be done using a method for sub_plane_to_screen. */
   void (*plane_to_screen)(splotd *, datad *, ggobid *);

   gint (*active_paint_points)(splotd *, datad *);


   GdkSegment *(*alloc_whiskers)(splotd *sp, gint nrows, datad *d);

	/** called from splot_plot_edge */
   gboolean (*draw_edge_p)(splotd *sp, gint m, datad *d, datad *e, ggobid *gg);
	/** called from splot_plot_case. Should probably be the same as draw_edge_p but
            doesn't take the edge argument! Could drop the first datad in splot_plot_edge
            and just hand it the one dataset. */
   gboolean (*draw_case_p)(splotd *sp, gint m, datad *d, ggobid *gg);

   void (*within_draw_to_binned)(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc);
   void (*within_draw_to_unbinned)(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc);

} GtkGGobiExtendedSPlotClass;

typedef struct 
{

   splotd splot; 

} extendedSPlotd;



void splot_init(splotd *sp, displayd *display, gint width, gint height, struct _ggobid *gg);
#endif
