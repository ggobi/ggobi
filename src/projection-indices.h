#ifndef PROJECTION_INDICES_H
#define PROJECTION_INDICES_H

#include "array.h"
#include "vector.h"


typedef gint (*Tour_PPIndex_f)(array_d *pd, void *params, gdouble *val, gpointer userData);

typedef struct {
    gchar *ppIndexName;   /* a string that can be used in the GUI to describe this PP index. */
    Tour_PPIndex_f index_f; /* The C routine that calculates the PP index value.*/
    gboolean checkGroups; /* Whether we have to call compute_groups and calculate the index only if this returns false(). */
    gpointer userData;    /* arbitrary data object that is passed in the call to index_f to parameterize it. */
} TourPPIndex;


typedef struct {
  vector_i ngroup, group; /* for class indices */
  gint numgroups; /* previously called groups in class code */
  array_d cov, tcov, mean; /* for lda, holes, cm */
  vector_d ovmean; /* for lda, holes, cm */
  vector_i nright, index; /* for gini, entropy */
  vector_d x; /* for gini, entropy */
} pp_param;
gint       alloc_pp (pp_param *pp, gint nrows, gint ncols, gint ndim);


/* Utility routines */
gdouble ludcmp(gdouble *a, gint n, gint *Pivot); 
gdouble tour_pp_solve(gdouble *a, gdouble *b, gint n, gint *Pivot); 
void inverse(gdouble *a, gint n);

/* Arbitrary dimensional indices */
void center (array_d *); 
gint ppi_pca (array_d *, void *, gdouble *,  gpointer userData);

gint ppi_holes(array_d *pdata, void *param, gdouble *val, gpointer unused);
gint ppi_central_mass(array_d *pdata, void *param, gdouble *val, gpointer unused);
void zero (gdouble *ptr, gint length);
void zero_int (gint *mem, gint size);
gint compute_groups (vector_i group, vector_i ngroup, gint *groups,  gint nrows, gdouble *gdata);
gint ppi_lda (array_d *pdata, void *param, gdouble *val, gpointer unused);
gint ppi_gini (array_d *pdata, void *param, gdouble *val, gpointer unused);
gint ppi_entropy (array_d *pdata, void *param, gdouble *val, gpointer unused);

#endif 
