#ifndef GGOBI_ARRAY_H
#define GGOBI_ARRAY_H

#ifdef GGOBIINTERN
#define GGOBI_
#else
#define GGOBI_ extern
#endif

#include "utils.h"

/*-- arrays --*/
/*-- double: some plugins will want these --*/
typedef struct {
  gdouble **vals;
  guint nrows, ncols;
} array_d;

/*-- short: for gg.missing --*/
typedef struct {
  gshort **vals;
  guint nrows, ncols;
} array_s;
/*-- long: for world --*/
typedef struct {
  glong **vals;
  guint nrows, ncols;
} array_l;


void       arrayd_add_cols (array_d *, gint);
void       arrayd_add_rows (array_d *, gint);
void       arrayd_alloc (array_d *, gint, gint);
void       arrayd_alloc_zero (array_d *, gint, gint);
void       arrayd_copy (array_d *, array_d *);
void       arrayd_delete_cols (array_d *, GSList *);
void       arrayd_delete_rows (array_d * arrp, GSList *rows);
void       arrayd_free (array_d *);
void       arrayd_init_null (array_d *);
void       arrayd_zero (array_d *);
void       arrayl_add_cols (array_l *, gint);
void       arrayl_add_rows (array_l *, gint);
void       arrayl_alloc (array_l *, gint, gint);
void       arrayl_alloc_zero (array_l *, gint, gint);
void       arrayl_delete_cols (array_l *, GSList *);
void       arrayl_delete_rows (array_l *, GSList *);
void       arrayl_free (array_l *, gint, gint);
void       arrayl_init_null (array_l *);
void       arrayl_zero (array_l *);
void       arrays_add_cols (array_s *, gint);
void       arrays_add_rows (array_s *, gint);
void       arrays_alloc (array_s *, gint, gint);
void       arrays_alloc_zero (array_s *, gint, gint);
void       arrays_delete_cols (array_s *, GSList *);
void       arrays_delete_rows (array_s *, GSList *);
void       arrays_free (array_s *, gint, gint);
void       arrays_init_null (array_s *);
void       arrays_zero (array_s *);


// Extras for vala compatability (probably better ways to do this)
array_d  *  arrayd_new (gint, gint);
gdouble ggobi_matrix_get_n_cols(array_d * arrp);
gdouble ggobi_matrix_get_n_rows(array_d * arrp);


#endif

