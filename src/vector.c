/*-- vector.c --*/

#include <gtk/gtk.h>
#include "vector.h"

/*-------------------------------------------------------------------------*/
/*                        double vector management                         */
/*-------------------------------------------------------------------------*/

void
vectord_init_null (vector_d *vecp)
{
  vecp->nels = 0;
  vecp->els = (gdouble *) NULL;
}
  
void
vectord_free (vector_d *vecp)
{
 if (vecp->els != NULL)
    g_free ((gpointer) vecp->els);
  vecp->els = NULL;
  vecp->nels = 0;
}

/* Zero a vector of doubles. */
void
vectord_zero (vector_d *vecp)
{
  gint i;
  for (i=0; i<vecp->nels; i++)
    vecp->els[i] = 0.0;
}

/* allocate a vector of doubles */
void
vectord_alloc (vector_d *vecp, gint nels)
{
  if (vecp->els != NULL)
    g_free (vecp->els);
  vecp->els = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->els = (gdouble *) g_malloc (nels * sizeof (gdouble));
}

void
vectord_realloc (vector_d *vecp, gint nels)
{
  if (nels > 0) {
    if (vecp->els == NULL || vecp->nels == 0)
      vecp->els = (gdouble *) g_malloc (nels * sizeof (gdouble));
    else {
      gint i, nels_prev = vecp->nels;
      vecp->els = (gdouble *) g_realloc (vecp->els, nels * sizeof (gdouble));
      /*-- initialize newly allocated slots to 0 --*/
      for (i=nels_prev; i<nels; i++)
        vecp->els[i] = 0.0;
    }
  } else {
    if (vecp->els != NULL)
      g_free (vecp->els);
    vecp->els = NULL;
  }

  vecp->nels = nels;
}

void
vectord_delete_els (vector_d *vecp, GSList *els)
{
  gint k;
  gint jto, jfrom;
  guint nkeepers;
  guint *keepers = find_keepers (vecp->nels, els, &nkeepers);

  if (els && nkeepers > 0) {

    /*-- copy before reallocating --*/
    for (k=0; k<nkeepers; k++) {
      jto = k;
      jfrom = keepers[k];  /*-- jto has to be less than jfrom --*/
      if (jto != jfrom)
        vecp->els[jto] = vecp->els[jfrom];
    }

    vecp->els = (gdouble *) g_realloc (vecp->els,
                                       nkeepers * sizeof (gdouble));
    vecp->nels = nkeepers;
  }
  g_free (keepers);
}

/* allocate a vector of doubles populated with 0 */
void
vectord_alloc_zero (vector_d *vecp, gint nels)
{
  if (vecp->els != NULL)
    g_free (vecp->els);
  vecp->els = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->els = (gdouble *) g_malloc0 (nels * sizeof (gdouble));
}

void
vectord_copy (vector_d *vecp_from, vector_d *vecp_to)
{
  gint i;

  if (vecp_from->nels == vecp_to->nels)
    for (i=0; i<vecp_from->nels; i++)
      vecp_to->els[i] = vecp_from->els[i];
  else
    g_printerr ("(vectord_copy) length of source = %d, of destination = %d\n",
      vecp_from->nels, vecp_to->nels);
}


/*-------------------------------------------------------------------------*/
/*                    integer vector management                            */
/*-------------------------------------------------------------------------*/

void
vectori_init_null (vector_i *vecp)
{
  vecp->nels = 0;
  vecp->els = (gint *) NULL;
}

void
vectori_free (vector_i *vecp)
{
  if (vecp->els != NULL)
    g_free ((gpointer) vecp->els);
  vecp->els = NULL;
  vecp->nels = 0;
}

/* Zero an integer vector. */
void
vectori_zero (vector_i *vecp)
{
  gint i;
  for (i=0; i<vecp->nels; i++)
    vecp->els[i] = 0;
}

/* allocate an integer vector */
void
vectori_alloc (vector_i *vecp, gint nels)
{
  if (vecp->els != NULL)
    g_free (vecp->els);
  vecp->els = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->els = (gint *) g_malloc (nels * sizeof (gint));
}

/* allocate an integer vector; populate with 0 */
void
vectori_alloc_zero (vector_i *vecp, gint nels)
{
  if (vecp->els != NULL)
    g_free (vecp->els);
  vecp->els = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->els = (gint *) g_malloc0 (nels * sizeof (gint));
}

void
vectori_realloc (vector_i *vecp, gint nels)
{
  if (nels > 0) {
    if (vecp->els == NULL || vecp->nels == 0)
      vecp->els = (gint *) g_malloc (nels * sizeof (gint));
    else {
      gint i, nels_prev = vecp->nels;
      vecp->els = (gint *) g_realloc (vecp->els, nels * sizeof (gint));
      /*-- initialize newly allocated slots to 0 --*/
      for (i=nels_prev; i<nels; i++)
        vecp->els[i] = 0;
    }
  } else {
    if (vecp->els != NULL)
      g_free (vecp->els);
    vecp->els = NULL;
  }

  vecp->nels = nels;
}

void
vectori_delete_els (vector_i *vecp, GSList *els)
{
  gint k;
  gint jto, jfrom;
  guint nkeepers;
  guint *keepers = find_keepers (vecp->nels, els, &nkeepers);

  if (els && nkeepers > 0) {

    /*-- copy before reallocating --*/
    for (k=0; k<nkeepers; k++) {
      jto = k;
      jfrom = keepers[k];  /*-- jto has to be less than jfrom --*/
      if (jto != jfrom)
        vecp->els[jto] = vecp->els[jfrom];
    }

    vecp->els = (gint *) g_realloc (vecp->els,
                                     nkeepers * sizeof (gint));
    vecp->nels = nkeepers;
  }
  g_free (keepers);
}

void
vectori_copy (vector_i *vecp_from, vector_i *vecp_to)
{
  gint i;

  if (vecp_from->nels == vecp_to->nels)
    for (i=0; i<vecp_from->nels; i++)
      vecp_to->els[i] = vecp_from->els[i];
  else
    g_printerr ("(vectori_copy) length of source = %d, of destination = %d\n",
      vecp_from->nels, vecp_to->nels);
}

/*-------------------------------------------------------------------------*/
/*                   gboolean vector management                            */
/*-------------------------------------------------------------------------*/

void vectorb_init_null (vector_b *vecp)
{
  vecp->nels = 0;
  vecp->els = (gboolean *) NULL;
}

void vectorb_free (vector_b *vecp)
{
  if (vecp->els == NULL)
    g_free ((gpointer) vecp->els);
  vecp->els = NULL;
  vecp->nels = 0;
}

void
vectorb_zero (vector_b *vecp)
{
  gint i;
  for (i=0; i<vecp->nels; i++)
    vecp->els[i] = FALSE;
}

void
vectorb_alloc (vector_b *vecp, gint nels)
{
  if (vecp->els != NULL)
    g_free (vecp->els);
  vecp->els = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->els = (gboolean *) g_malloc (nels * sizeof (gboolean));
}

void
vectorb_realloc (vector_b *vecp, gint nels)
{
  if (nels > 0) {
    if (vecp->els == NULL || vecp->nels == 0)
      vecp->els = (gboolean *) g_malloc (nels * sizeof (gboolean));
    else {
      gint i, nels_prev = vecp->nels;
      vecp->els = (gboolean *)
        g_realloc (vecp->els, nels * sizeof (gboolean));
      /*-- initialize newly allocated slots to false --*/
      for (i=nels_prev; i<nels; i++)
        vecp->els[i] = FALSE;
    }
  } else {
    if (vecp->els != NULL)
      g_free (vecp->els);
    vecp->els = NULL;
  }

  vecp->nels = nels;
}

void
vectorb_alloc_zero (vector_b *vecp, gint nels)
{
  if (vecp->els != NULL)
    g_free (vecp->els);
  vecp->els = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->els = (gboolean *) g_malloc0 (nels * sizeof (gboolean));
}

void
vectorb_delete_els (vector_b *vecp, GSList *els)
{
  gint k;
  gint jto, jfrom;
  guint nkeepers;
  guint *keepers = find_keepers (vecp->nels, els, &nkeepers);

  if (els && nkeepers > 0) {

    /*-- copy before reallocating --*/
    for (k=0; k<nkeepers; k++) {
      jto = k;
      jfrom = keepers[k];  /*-- jto has to be less than jfrom --*/
      if (jto != jfrom)
        vecp->els[jto] = vecp->els[jfrom];
    }

    vecp->els = (gboolean *) g_realloc (vecp->els,
                                     nkeepers * sizeof (gboolean));
    vecp->nels = nkeepers;
  }
  g_free (keepers);
}

void
vectorb_copy (vector_b *vecp_from, vector_b *vecp_to)
{
  gint i;

  if (vecp_from->nels == vecp_to->nels)
    for (i=0; i<vecp_from->nels; i++)
      vecp_to->els[i] = vecp_from->els[i];
  else
    g_printerr ("(vectorb_copy) length of source = %d, of destination = %d\n",
      vecp_from->nels, vecp_to->nels);
}

/*-------------------------------------------------------------------------*/
/*                     gshort vector management                            */
/*-------------------------------------------------------------------------*/

void vectors_init_null (vector_s *vecp)
{
  vecp->nels = 0;
  vecp->els = (gshort *) NULL;
}

void vectors_free (vector_s *vecp)
{
  if (vecp->els == NULL)
    g_free ((gpointer) vecp->els);
  vecp->els = NULL;
  vecp->nels = 0;
}

void vectors_zero (vector_s *vecp)
{
  gint i;
  for (i=0; i<vecp->nels; i++)
    vecp->els[i] = 0;
}

void
vectors_alloc (vector_s *vecp, gint nels)
{
  if (vecp->els != NULL)
    g_free (vecp->els);
  vecp->els = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->els = (gshort *) g_malloc (nels * sizeof (gshort));
}

void
vectors_realloc (vector_s *vecp, gint nels)
{
  if (nels > 0) {
    if (vecp->els == NULL || vecp->nels == 0)
      vecp->els = (gshort *) g_malloc (nels * sizeof (gshort));
    else {
      gint i, nels_prev = vecp->nels;
      vecp->els = (gshort *) g_realloc (vecp->els, nels * sizeof (gshort));
      /*-- initialize newly allocated slots to 0 --*/
      for (i=nels_prev; i<nels; i++)
        vecp->els[i] = 0;
    }
  } else {
    if (vecp->els != NULL)
      g_free (vecp->els);
    vecp->els = NULL;
  }

  vecp->nels = nels;
}

void
vectors_alloc_zero (vector_s *vecp, gint nels)
{
  if (vecp->els != NULL)
    g_free (vecp->els);
  vecp->els = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->els = (gshort *) g_malloc0 (nels * sizeof (gshort));
}

void
vectors_copy (vector_s *vecp_from, vector_s *vecp_to)
{
  gint i;

  if (vecp_from->nels == vecp_to->nels)
    for (i=0; i<vecp_from->nels; i++)
      vecp_to->els[i] = vecp_from->els[i];
  else
    g_printerr ("(vectors_copy) length of source = %d, of destination = %d\n",
      vecp_from->nels, vecp_to->nels);
}
