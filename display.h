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
 datad *e;  /*-- pointer to a particular gg->d[] --*/

/* Missing values */
 gboolean missing_p;  /* false by default */

/*
 * Actually, this might need to be a pair of vectors
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
  gint ntour_vars; 
  vector_i tour_vars;
  array_f u0, u1, u, uold, v0, v1, v, uvevec, tv;
  vector_f lambda, tau, tinc;
  gfloat dv, delta;
  /*  gfloat ts[2], coss[2], sins[2];
  gint icoss[2], isins[2];*//* di - not sure i need this in the new code.*/
  gint tour_idled;
  gboolean tour_get_new_target;
  gint tour_nsteps, tour_stepcntr;

/*
 * Correlation Tour
*/
  array_f tc1_manbasis, tc2_manbasis;
  gint tc1_manip_var, tc2_manip_var;
  gint tc1_pos_old, tc1_pos, tc2_pos_old, tc2_pos;
  gint tc_manip_mode;
  gboolean tc1_manipvar_inc, tc2_manipvar_inc;
  gfloat tc1_phi, tc2_phi;

/*
 * 2d tour
*/
 tour t2d;

/*
 * 1d tour
*/
 tour t1d;

/*
 * corr tour
*/
 tour tcorr1, tcorr2;

 struct _ggobid *ggobi;

};  /* displayd; */


gboolean isEmbeddedDisplay(displayd *dpy);


#endif
