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

typedef struct {

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

/*
 * barcharts and histograms
*/
 barchartd *bar;

#ifdef WIN32
#endif
/*
 * for Windows drawing -- this or GArrays?  
 * Do I really need to keep doing this?
 struct _win32 {
   struct {GdkPoint *data; gint n;} points;
   struct {rectd *data; gint n;} orects;
   struct {rectd *data; gint n;} frects;
   struct {arcd *data; gint n;} oarcs;
   struct {arcd *data; gint n;} farcs;
   struct {GdkSegment *data; gint n;} segs;
   struct {GdkSegment *data; gint n;} whisker_segs;
   struct {GdkSegment *data; gint n;} ash_segs;
 } win32;
*/

} splotd;


#endif
