/*-- display.h --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#ifndef DISPLAY_H
#define DISPLAY_H

#include "defines.h"
#include "cpanel.h"
#include "splot.h"
#include "datad.h"

#include "ggobi.h"

struct _ggobid; 
typedef struct _ggobid ggobid;

struct _displayd {

/*
 * Used by all displays
*/
 GtkWidget *window;
 cpaneld cpanel;
 enum displaytyped displaytype;

 struct _displayd *embeddedIn;

 GList *splots;  /* doubly linked list of splots */

 datad *d;  /*-- pointer to a particular gg->d[] --*/

/* Missing values */
 gboolean missing_p;  /* false by default */

/*
 * Actually, this is going to need to be a pair of vectors
 * or linked lists, corresponding to the number of plots in
 * the display.  But let's be lazy for the moment.
*/
 GtkWidget *hrule, *vrule;

 DisplayOptions options;


/*
 * For an individual scatterplot
*/


/*
 * Scatterplot matrix display
*/
 GList *scatmat_cols, *scatmat_rows;
 GtkWidget *table;


/*
 * Parallel coordinates display
*/
  gint p1d_orientation;

/*
 * Tour display
 */
  gint ntour_vars, *tour_vars;
  gfloat **u0, **u1, **u, **uold, **v0, **v1, **v, **uvevec;
  gfloat *lambda, *tau, *tinc, dv, delta;
  gfloat **tv;
  /*  gfloat ts[2], coss[2], sins[2];
  gint icoss[2], isins[2];*//* di - not sure i need this in the new code.*/
  gint tour_idled;
  gboolean tour_get_new_target;
  gint tour_nsteps, tour_stepcntr;

/*
 * Correlation Tour
*/
  gint ncorrvars_x, *corrvars_x, ncorrvars_y, *corrvars_y;

  struct _ggobid *ggobi;

};  /* displayd; */


gboolean isEmbeddedDisplay(displayd *dpy);


#endif
