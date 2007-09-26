#ifndef PROJECTION_OPTIMISATION_H
#define PROJECTION_OPTIMISATION_H

#include <glib.h>
#include "array.h"
#include "projection-indices.h"

typedef struct { 
  gdouble temp_start, temp_end, cooling, heating, temp, index_best;
  gint restart, maxproj, success;
  array_d proj_best, data, pdata;
} optimize0_param;

gint alloc_optimize0_p (optimize0_param *op, gint nrows, gint ncols, gint ndim);
gint free_optimize0_p (optimize0_param *op);
gint optimize0 (optimize0_param *op, Tour_PPIndex_f fun, void *param);
gint realloc_optimize0_p (optimize0_param *, gint, vector_i);

gboolean iszero (array_d *data);
void normal_fill (array_d *data, gdouble delta, array_d *base);
void orthonormal (array_d *proj);

#endif 
