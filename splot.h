/*-- splot.h --*/
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

 gint redraw_style;

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
 icoords ishift;

/*
 * button and key event information
*/
 gint motion_id, press_id, release_id, key_press_id;
 icoords mousepos, mousepos_o;

/*
 * plot1d  (used in parcoords as well as scatterplot)
*/
 gint p1dvar;
 vector_f p1d_data; /* the spreading data */
 /*gfloat *p1d_data;*/ 
 lims p1d_lim;      /* limits of the spreading data */
 gfloat p1d_mean;

/*
 * xyplot
*/
 icoords xyvars;

/*
 * rotation
*/
 struct {gint x, y, z;} spinvars;

/*
 * tour
*/

/*
 * correlation tour
*/

/*
 * brushing
*/

} splotd;


#endif
