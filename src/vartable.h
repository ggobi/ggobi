/*-- vartable.h --*/

#ifndef VARTABLE_H
#define VARTABLE_H

#include "types.h"

enum { 
  VT_VARNAME, VT_TFORM,
  VT_REAL_USER_MIN, VT_REAL_USER_MAX,
  VT_REAL_DATA_MIN, VT_REAL_DATA_MAX,
  VT_MEAN, VT_MEDIAN,
  VT_NLEVELS, VT_LEVEL_NAME, VT_LEVEL_VALUE, VT_LEVEL_COUNT,
  VT_CAT_USER_MIN, VT_CAT_USER_MAX,
  VT_CAT_DATA_MIN, VT_CAT_DATA_MAX,
  VT_NMISSING,
  NCOLS_VT
};

typedef enum {ADDVAR_ROWNOS, ADDVAR_BGROUP} NewVariableType; 


typedef enum {real, categorical, integer, counter, uniform, all_vartypes} vartyped;

/*
 * A vartabled object is not a table, but rather an entry in a table:
 * it's all the data for a single variable, and it is used to populate
 * a row in the variable manipulation table.  Now that is done using a
 * GtkTreeModel, so be careful before adding or moving an element in
 * this structure.
*/
typedef struct {
  GObject *d;  /*-- the parent datad --*/

  gchar *collab, *collab_tform;
  gchar *nickname;   /*-- very short name to use in tour axis labels --*/

  /*-- is this variable categorical? --*/
  vartyped vartype;
  gboolean isTime;

  /*-- categorical_p --*/
  gint nlevels;
  gint *level_values;
  gint *level_counts;
  gchar **level_names;  /*-- strings --*/

  /*-- unadjusted, unaffected by imputation --*/
  gfloat mean, median;

  /* Limits
    ================================================= */

  lims lim;             /*used: lim_specified_tform or lim_tform*/
  lims lim_raw;         /* raw data*/
  lims lim_tform;       /*transformed data*/
  lims lim_display;     /*for vartable, transformed sans missings*/
  lims lim_specified;   /*user specified*/
  lims lim_specified_tform;
  gboolean lim_specified_p;

  /* Transformations
     =================================================*/

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
