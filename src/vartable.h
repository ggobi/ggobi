/*-- vartable.h --*/

#ifndef VARTABLE_H
#define VARTABLE_H

enum { 
      VT_VARNAME, VT_TFORM,
      VT_REAL_USER_MIN, VT_REAL_USER_MAX,
      VT_REAL_DATA_MIN, VT_REAL_DATA_MAX,
      VT_MEAN, VT_MEDIAN,
	  VT_NLEVELS, VT_LEVEL_NAME, VT_LEVEL_VALUE, VT_LEVEL_COUNT,
      VT_CAT_USER_MIN, VT_CAT_USER_MAX,
      VT_CAT_DATA_MIN, VT_CAT_DATA_MAX,
      VT_NMISSING,
      NCOLS_VT};

typedef enum {ADDVAR_ROWNOS = 0, ADDVAR_BGROUP} NewVariableType; 

extern const double AddVarRowNumbers;
extern const double AddVarBrushGroup;

typedef enum {real, categorical, integer, counter, uniform,  all_vartypes} vartyped;


/*
 * a vartabled object is not a table, but rather an entry in
 * a table:  it's all the data for a single variable, and it
 * is used to populate a row in the variable manipulation table.
*/

typedef struct {
 struct _datad *d;  /*-- I want a pointer back to its parent datad --*/

 gchar *collab, *collab_tform;
 gchar *nickname;   /*-- very short name to use in tour axis labels --*/
 gint nmissing;

 /*-- is this variable categorical? --*/
 vartyped vartype;
 gboolean isTime;

 /*-- categorical_p --*/
 gint nlevels;
 gint *level_values;
 gint *level_counts;
 gchar **level_names;  /*-- strings --*/

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
