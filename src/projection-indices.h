#ifndef PROJECTION_INDICES_H
#define PROJECTION_INDICES_H

#include "array.h"
#include "vector.h"


typedef gint (*PPIndex)(array_d *pd, void *params, gdouble *val);

typedef struct {
    gchar *ppIndexName;   /* a string that can be used in the GUI to describe this PP index. */
    PPIndex index_f; /* The C routine that calculates the PP index value.*/
    gboolean checkGroups; /* Whether we have to call compute_groups and calculate the index only if this returns false(). */
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


typedef enum {HOLES, CENTRAL_MASS, LDA, CGINI, CENTROPY} StandardPPIndexTypes;


/* Arbitrary dimensional indices */
gint ppi_pca          (array_d *pdata, void *param, gdouble *val);
gint ppi_holes        (array_d *pdata, void *param, gdouble *val);
gint ppi_central_mass (array_d *pdata, void *param, gdouble *val);
gint ppi_lda          (array_d *pdata, void *param, gdouble *val);
gint ppi_gini         (array_d *pdata, void *param, gdouble *val);
gint ppi_entropy      (array_d *pdata, void *param, gdouble *val);

gint compute_groups (vector_i group, vector_i ngroup, gint *groups,  gint nrows, gdouble *gdata);


#endif 
