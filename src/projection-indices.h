#ifndef PROJECTION_INDICES_H
#define PROJECTION_INDICES_H

#include "array.h"
#include "vector.h"


/* Arbitrary dimensional indices */
typedef 
gdouble (*PPIndex)       (array_d data, vector_d groups);
gdouble ppi_pca          (array_d data, vector_d groups);
gdouble ppi_holes        (array_d data, vector_d groups);
gdouble ppi_central_mass (array_d data, vector_d groups);
gdouble ppi_lda          (array_d data, vector_d groups);

typedef struct {
    gchar *ppIndexName;   /* a string that can be used in the GUI to describe this PP index. */
    PPIndex index_f; /* The C routine that calculates the PP index value.*/
} TourPPIndex;

typedef enum {HOLES, CENTRAL_MASS, LDA, PCA} StandardPPIndexTypes;


#endif 
