#ifndef GGOBI_ARRAY_H
#define GGOBI_ARRAY_H

#ifdef GGOBIINTERN
#define GGOBI_
#else
#define GGOBI_ extern
#endif

typedef gfloat greal;

/*-- arrays --*/
/*-- double: some plugins will want these --*/
typedef struct {
  gdouble **vals;
  guint nrows, ncols;
} array_d;
/*-- floating point: for gg.raw_data, tform1, tform2 --*/
typedef struct {
  gfloat **vals;
  guint nrows, ncols;
} array_f;
/*-- short: for gg.missing --*/
typedef struct {
  gshort **vals;
  guint nrows, ncols;
} array_s;
/*-- long: for world, jitdata --*/
typedef struct {
  glong **vals;
  guint nrows, ncols;
} array_l;
/*-- real: for the new world, jitdata --*/
typedef struct {
  greal **vals;
  guint nrows, ncols;
} array_g;


void       arrayd_add_cols (array_d *, gint);
void       arrayd_add_rows (array_d *, gint);
array_d  *  arrayd_new (gint, gint);
void       arrayd_alloc (array_d *, gint, gint);
void       arrayd_alloc_zero (array_d *, gint, gint);
void       arrayd_copy (array_d *, array_d *);
void       arrayd_delete_cols (array_d *, GSList *);
void       arrayd_delete_rows (array_d * arrp, GSList *rows);
void       arrayd_free (array_d *);
void       arrayd_init_null (array_d *);
void       arrayd_zero (array_d *);
void       arrayf_add_cols (array_f *, gint);
void       arrayf_add_rows (array_f *, gint);
void       arrayf_alloc (array_f *, gint, gint);
void       arrayf_alloc_zero (array_f *, gint, gint);
void       arrayf_copy (array_f *, array_f *);
void       arrayf_delete_cols (array_f *, GSList *);
void       arrayf_free (array_f *, gint, gint);
void       arrayf_init_null (array_f *);
void       arrayf_zero (array_f *);
void       arrayg_add_cols (array_g *, gint);
void       arrayg_add_rows (array_g *, gint);
void       arrayg_alloc (array_g *, gint, gint);
void       arrayg_alloc_zero (array_g *, gint, gint);
void       arrayg_delete_cols (array_g *, GSList *);
void       arrayg_delete_rows (array_g *, GSList *);
void       arrayg_free (array_g *, gint, gint);
void       arrayg_init_null (array_g *);
void       arrayg_zero (array_g *);
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

guint      *find_keepers (gint ncols_current, GSList *cols, guint *nkeepers);

#endif

