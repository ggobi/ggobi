/*-- vector.c --*/

#include <gtk/gtk.h>
#include <vars.h>


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
