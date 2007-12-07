#ifndef GGOBI_VECTOR_H
#define GGOBI_VECTOR_H

#include "utils.h"

typedef struct {
  gdouble *els;
  guint nels;
} vector_d;
typedef struct {
  gint *els;
  guint nels;
} vector_i;
typedef struct {
  gshort *els;
  guint nels;
} vector_s;
typedef struct {
  gboolean *els;
  guint nels;
} vector_b;


void       vectorb_alloc (vector_b *, gint);
void       vectorb_alloc_zero (vector_b *, gint);
void       vectorb_copy (vector_b *, vector_b *);
void       vectorb_delete_els (vector_b *vecp, GSList *els);
void       vectorb_free (vector_b *);
void       vectorb_init_null (vector_b *);
void       vectorb_realloc (vector_b *, gint);
void       vectorb_realloc_zero (vector_b *, gint);
void       vectorb_zero (vector_b *vecp);
void       vectord_alloc (vector_d *, gint);
void       vectord_alloc_zero (vector_d *, gint);
void       vectord_delete_els (vector_d *vecp, GSList *els);
void       vectord_free (vector_d *);
void       vectord_init_null (vector_d *);
void       vectord_realloc (vector_d *, gint);
void       vectord_zero (vector_d *vecp);
void       vectori_alloc (vector_i *, gint);
void       vectori_alloc_zero (vector_i *, gint);
void       vectori_copy (vector_i *, vector_i *);
void       vectori_delete_els (vector_i *vecp, GSList *els);
void       vectori_free (vector_i *);
void       vectori_init_null (vector_i *);
void       vectori_realloc (vector_i *, gint);
void       vectori_zero (vector_i *vecp);
void       vectors_copy (vector_s *, vector_s *);
void       vectors_free (vector_s *);
void       vectors_init_null (vector_s *);
void       vectors_realloc (vector_s *, gint);
void       vectors_realloc_zero (vector_s *, gint);

#endif
