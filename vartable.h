/*-- vartable.h --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#ifndef VARTABLE_H
#define VARTABLE_H

#if 0
#define NCOLS_CLIST 11
#endif

enum {CLIST_VARNO, CLIST_VARNAME, CLIST_TYPE, CLIST_TFORM,
      CLIST_USER_MIN, CLIST_USER_MAX, CLIST_DATA_MIN, CLIST_DATA_MAX,
      CLIST_MEAN, CLIST_MEDIAN, CLIST_NMISSING, NCOLS_CLIST};

#define ADDVAR_ROWNOS   0
#define ADDVAR_BGROUP   1

/*
 * a vartabled object is not a table, but rather an entry in
 * a table:  it's all the data for a single variable, and it
 * is used to populate a row in the variable manipulation table.
*/

typedef struct {
 gchar *collab, *collab_tform;
 gint nmissing;

 /*-- is this variable categorical? --*/
 gboolean categorical_p;
 gint nlevels;
 GArray *levels;

 /*-- reference variable:  jref=-1 except for cloned or sphered variables --*/
 gint jref;

 /*-- unadjusted, unaffected by imputation --*/
 gfloat mean, median;

 lims lim_raw;       /*-- range of the raw data          --*/
 lims lim_tform;     /*-- range of d->tform              --*/

 /*
  * the limits to be put into the table: presently, this is
  * lim_tform but it excludes any missing values.
 */
 lims lim_display;  

 /*
  * If the user has supplied limits, lim_specified_p = true
  * and the limits are stored in lim_specified.{min,max}
 */
 gboolean lim_specified_p;
 lims lim_specified;
 lims lim_specified_tform;

 lims lim;      /*-- limits in use: lim_specified_tform or lim_tform --*/

 /*-- transformations --*/
 gint tform0;
 gfloat domain_incr;  /*-- stage 0 --*/
 gfloat (*domain_adj) (gfloat x, gfloat incr);
 gfloat (*inv_domain_adj) (gfloat x, gfloat incr);
 gint tform1;
 gfloat param;
 gint tform2;

 /*-- jittering --*/
 gfloat jitter_factor;

 /*-- in variable table --*/
 gboolean selected;

} vartabled;

#endif
