/*-- vector.c --*/

#include <gtk/gtk.h>
#include <vars.h>

#include "externs.h"


/*-------------------------------------------------------------------------*/
/*                    floating point vector management                     */
/*-------------------------------------------------------------------------*/

void
vectorf_init (vector_f *vecp)
{
  vecp->nels = 0;
  vecp->data = (gfloat *) NULL;
}

void
vectorf_free (vector_f *vecp)
{
  g_free ((gpointer) vecp);
  vecp->nels = 0;
}

/* Zero a floating point vector. */
void
vectorf_zero (vector_f *vecp)
{
  gint i;
  for (i=0; i<vecp->nels; i++)
    vecp->data[i] = 0.0;
}

/* allocate or reallocate a floating point vector */
void
vectorf_realloc (vector_f *vecp, gint nels)
{
  vecp->data = (gfloat *) g_realloc (vecp->data, nels * sizeof (gfloat));
  vecp->nels = nels;
}

/* allocate or reallocate a floating point vector populated with 0 */
void
vectorf_realloc_zero (vector_f *vecp, gint nels)
{
  vecp->data = (gfloat *) g_realloc (vecp->data, nels * sizeof (gfloat));
  vecp->nels = nels;

  vectorf_zero (vecp);
}

void
vectorf_copy (vector_f *vecp_from, vector_f *vecp_to)
{
  gint i;

  if (vecp_from->nels == vecp_to->nels)
    for (i=0; i<vecp_from->nels; i++)
      vecp_to->data[i] = vecp_from->data[i];
}

/*-------------------------------------------------------------------------*/
/*                    integer vector management                            */
/*-------------------------------------------------------------------------*/

void
vectori_init (vector_i *vecp)
{
  vecp->nels = 0;
  vecp->data = (gint *) NULL;
}

void
vectori_free (vector_i *vecp)
{
  g_free ((gpointer) vecp);
  vecp->nels = 0;
}

/* Zero an integer vector. */
void
vectori_zero (vector_i *vecp)
{
  gint i;
  for (i=0; i<vecp->nels; i++)
    vecp->data[i] = 0;
}

/* allocate or reallocate an integer vector */
void
vectori_realloc (vector_i *vecp, gint nels)
{
  vecp->data = (gint *) g_realloc (vecp->data, nels * sizeof (gint));
  vecp->nels = nels;
}

/* allocate or reallocate an integer vector; populate with 0 */
void
vectori_realloc_zero (vector_i *vecp, gint nels)
{
  vecp->data = (gint *) g_realloc (vecp->data, nels * sizeof (gint));
  vecp->nels = nels;

  vectori_zero (vecp);
}

void
vectori_copy (vector_i *vecp_from, vector_i *vecp_to)
{
  gint i;

  if (vecp_from->nels == vecp_to->nels)
    for (i=0; i<vecp_from->nels; i++)
      vecp_to->data[i] = vecp_from->data[i];
}

/*-------------------------------------------------------------------------*/
/*                   gboolean vector management                            */
/*-------------------------------------------------------------------------*/

void vectorb_init (vector_b *vecp)
{
  vecp->nels = 0;
  vecp->data = (gboolean *) NULL;
}

void vectorb_free (vector_b *vecp)
{
  g_free ((gpointer) vecp);
  vecp->nels = 0;
}

void vectorb_zero (vector_b *vecp)
{
  gint i;
  for (i=0; i<vecp->nels; i++)
    vecp->data[i] = false;
}

void vectorb_realloc (vector_b *vecp, gint nels)
{
  vecp->data = (gboolean *)
    g_realloc (vecp->data, nels * sizeof (gboolean));
  vecp->nels = nels;
}

void vectorb_realloc_zero (vector_b *vecp, gint nels)
{
  vecp->data = (gboolean *) g_realloc (vecp->data, nels * sizeof (gboolean));
  vecp->nels = nels;
  vectorb_zero (vecp);
}

void vectorb_copy (vector_b *vecp_from, vector_b *vecp_to)
{
  gint i;
  if (vecp_from->nels == vecp_to->nels)
    for (i=0; i<vecp_from->nels; i++)
      vecp_to->data[i] = vecp_from->data[i];
}

/*-------------------------------------------------------------------------*/
/*                     gshort vector management                            */
/*-------------------------------------------------------------------------*/

void vectors_init (vector_s *vecp)
{
  vecp->nels = 0;
  vecp->data = (gshort *) NULL;
}

void vectors_free (vector_s *vecp)
{
  g_free ((gpointer) vecp);
  vecp->nels = 0;
}

void vectors_zero (vector_s *vecp)
{
  gint i;
  for (i=0; i<vecp->nels; i++)
    vecp->data[i] = 0;
}

void vectors_realloc (vector_s *vecp, gint nels)
{
  vecp->data = (gshort *)
    g_realloc (vecp->data, nels * sizeof (gshort));
  vecp->nels = nels;
}

void vectors_realloc_zero (vector_s *vecp, gint nels)
{
  vecp->data = (gshort *) g_realloc (vecp->data, nels * sizeof (gshort));
  vecp->nels = nels;
  vectors_zero (vecp);
}

void vectors_copy (vector_s *vecp_from, vector_s *vecp_to)
{
  gint i;
  if (vecp_from->nels == vecp_to->nels)
    for (i=0; i<vecp_from->nels; i++)
      vecp_to->data[i] = vecp_from->data[i];
}
