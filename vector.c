/*-- vector.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

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
  vecp->vals = (gfloat *) NULL;
}

void
vectorf_free (vector_f *vecp)
{
 if (vecp->vals != NULL)
    g_free ((gpointer) vecp->vals);
  vecp->vals = NULL;
  vecp->nels = 0;
}

/* Zero a floating point vector. */
void
vectorf_zero (vector_f *vecp)
{
  gint i;
  for (i=0; i<vecp->nels; i++)
    vecp->vals[i] = 0.0;
}

/* allocate a floating point vector */
void
vectorf_alloc (vector_f *vecp, gint nels)
{
  if (vecp->vals != NULL)
    g_free (vecp->vals);
  vecp->vals = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->vals = (gfloat *) g_malloc (nels * sizeof (gfloat));
}

void
vectorf_realloc (vector_f *vecp, gint nels)
{
  if (nels > 0) {
    if (vecp->vals == NULL)
      vecp->vals = (gfloat *) g_malloc (nels * sizeof (gfloat));
    else 
      vecp->vals = (gfloat *) g_realloc (vecp->vals, nels * sizeof (gfloat));
  } else {
    if (vecp->vals != NULL)
      g_free (vecp->vals);
    vecp->vals = NULL;
  }

  vecp->nels = nels;
}

void
vectorf_delete_els (vector_f *vecp, gint nels, gint *els)
{
  gint k;
  gint jto, jfrom;
  gint *keepers = g_malloc ((vecp->nels - nels) * sizeof (gint));
  gint nkeepers = find_keepers (vecp->nels, nels, els, keepers);

  if (nels > 0 && nkeepers > 0) {

    /*-- copy before reallocating --*/
    for (k=0; k<nkeepers; k++) {
      jto = k;
      jfrom = keepers[k];  /*-- jto has to be less than jfrom --*/
      if (jto != jfrom)
        vecp->vals[jto] = vecp->vals[jfrom];
    }

    vecp->vals = (gfloat *) g_realloc (vecp->vals,
                                       nkeepers * sizeof (gfloat));
    vecp->nels = nkeepers;
  }
  g_free (keepers);
}

/* allocate a floating point vector populated with 0 */
void
vectorf_alloc_zero (vector_f *vecp, gint nels)
{
  if (vecp->vals != NULL)
    g_free (vecp->vals);
  vecp->vals = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->vals = (gfloat *) g_malloc0 (nels * sizeof (gfloat));
}

void
vectorf_copy (vector_f *vecp_from, vector_f *vecp_to)
{
  gint i;

  if (vecp_from->nels == vecp_to->nels)
    for (i=0; i<vecp_from->nels; i++)
      vecp_to->vals[i] = vecp_from->vals[i];
  else
    g_printerr ("(vectorf_copy) length of source = %d, of destination = %d\n",
      vecp_from->nels, vecp_to->nels);
}

/*-------------------------------------------------------------------------*/
/*                    integer vector management                            */
/*-------------------------------------------------------------------------*/

void
vectori_init (vector_i *vecp)
{
  vecp->nels = 0;
  vecp->vals = (gint *) NULL;
}

void
vectori_free (vector_i *vecp)
{
  if (vecp->vals != NULL)
    g_free ((gpointer) vecp->vals);
  vecp->vals = NULL;
  vecp->nels = 0;
}

/* Zero an integer vector. */
void
vectori_zero (vector_i *vecp)
{
  gint i;
  for (i=0; i<vecp->nels; i++)
    vecp->vals[i] = 0;
}

/* allocate an integer vector */
void
vectori_alloc (vector_i *vecp, gint nels)
{
  if (vecp->vals != NULL)
    g_free (vecp->vals);
  vecp->vals = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->vals = (gint *) g_malloc (nels * sizeof (gint));
}

/* allocate an integer vector; populate with 0 */
void
vectori_alloc_zero (vector_i *vecp, gint nels)
{
  if (vecp->vals != NULL)
    g_free (vecp->vals);
  vecp->vals = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->vals = (gint *) g_malloc0 (nels * sizeof (gint));
}

void
vectori_realloc (vector_i *vecp, gint nels)
{
  if (nels > 0) {
    if (vecp->vals == NULL)
      vecp->vals = (gint *) g_malloc (nels * sizeof (gint));
    else 
      vecp->vals = (gint *) g_realloc (vecp->vals, nels * sizeof (gint));
  } else {
    if (vecp->vals != NULL)
      g_free (vecp->vals);
    vecp->vals = NULL;
  }

  vecp->nels = nels;
}

void
vectori_delete_els (vector_i *vecp, gint nels, gint *els)
{
  gint k;
  gint jto, jfrom;
  gint *keepers = g_malloc ((vecp->nels - nels) * sizeof (gint));
  gint nkeepers = find_keepers (vecp->nels, nels, els, keepers);

  if (nels > 0 && nkeepers > 0) {

    /*-- copy before reallocating --*/
    for (k=0; k<nkeepers; k++) {
      jto = k;
      jfrom = keepers[k];  /*-- jto has to be less than jfrom --*/
      if (jto != jfrom)
        vecp->vals[jto] = vecp->vals[jfrom];
    }

    vecp->vals = (gint *) g_realloc (vecp->vals,
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
      vecp_to->vals[i] = vecp_from->vals[i];
  else
    g_printerr ("(vectori_copy) length of source = %d, of destination = %d\n",
      vecp_from->nels, vecp_to->nels);
}

/*-------------------------------------------------------------------------*/
/*                   gboolean vector management                            */
/*-------------------------------------------------------------------------*/

void vectorb_init (vector_b *vecp)
{
  vecp->nels = 0;
  vecp->vals = (gboolean *) NULL;
}

void vectorb_free (vector_b *vecp)
{
  if (vecp->vals == NULL)
    g_free ((gpointer) vecp->vals);
  vecp->vals = NULL;
  vecp->nels = 0;
}

void
vectorb_zero (vector_b *vecp)
{
  gint i;
  for (i=0; i<vecp->nels; i++)
    vecp->vals[i] = false;
}

void
vectorb_alloc (vector_b *vecp, gint nels)
{
  if (vecp->vals != NULL)
    g_free (vecp->vals);
  vecp->vals = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->vals = (gboolean *) g_malloc (nels * sizeof (gboolean));
}

void
vectorb_realloc (vector_b *vecp, gint nels)
{
  if (nels > 0) {
    if (vecp->vals == NULL)
      vecp->vals = (gboolean *) g_malloc (nels * sizeof (gboolean));
    else 
      vecp->vals = (gboolean *)
        g_realloc (vecp->vals, nels * sizeof (gboolean));
  } else {
    if (vecp->vals != NULL)
      g_free (vecp->vals);
    vecp->vals = NULL;
  }

  vecp->nels = nels;
}

void
vectorb_alloc_zero (vector_b *vecp, gint nels)
{
  if (vecp->vals != NULL)
    g_free (vecp->vals);
  vecp->vals = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->vals = (gboolean *) g_malloc0 (nels * sizeof (gboolean));
}

void
vectorb_copy (vector_b *vecp_from, vector_b *vecp_to)
{
  gint i;

  if (vecp_from->nels == vecp_to->nels)
    for (i=0; i<vecp_from->nels; i++)
      vecp_to->vals[i] = vecp_from->vals[i];
  else
    g_printerr ("(vectorb_copy) length of source = %d, of destination = %d\n",
      vecp_from->nels, vecp_to->nels);
}

/*-------------------------------------------------------------------------*/
/*                     gshort vector management                            */
/*-------------------------------------------------------------------------*/

void vectors_init (vector_s *vecp)
{
  vecp->nels = 0;
  vecp->vals = (gshort *) NULL;
}

void vectors_free (vector_s *vecp)
{
  if (vecp->vals == NULL)
    g_free ((gpointer) vecp->vals);
  vecp->vals = NULL;
  vecp->nels = 0;
}

void vectors_zero (vector_s *vecp)
{
  gint i;
  for (i=0; i<vecp->nels; i++)
    vecp->vals[i] = 0;
}

void
vectors_alloc (vector_s *vecp, gint nels)
{
  if (vecp->vals != NULL)
    g_free (vecp->vals);
  vecp->vals = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->vals = (gshort *) g_malloc (nels * sizeof (gshort));
}

void
vectors_realloc (vector_s *vecp, gint nels)
{
  if (nels > 0) {
    if (vecp->vals == NULL)
      vecp->vals = (gshort *) g_malloc (nels * sizeof (gshort));
    else 
      vecp->vals = (gshort *) g_realloc (vecp->vals, nels * sizeof (gshort));
  } else {
    if (vecp->vals != NULL)
      g_free (vecp->vals);
    vecp->vals = NULL;
  }

  vecp->nels = nels;
}

void
vectors_alloc_zero (vector_s *vecp, gint nels)
{
  if (vecp->vals != NULL)
    g_free (vecp->vals);
  vecp->vals = NULL;

  vecp->nels = nels;
  if (nels > 0)
    vecp->vals = (gshort *) g_malloc0 (nels * sizeof (gshort));
}

void
vectors_copy (vector_s *vecp_from, vector_s *vecp_to)
{
  gint i;

  if (vecp_from->nels == vecp_to->nels)
    for (i=0; i<vecp_from->nels; i++)
      vecp_to->vals[i] = vecp_from->vals[i];
  else
    g_printerr ("(vectors_copy) length of source = %d, of destination = %d\n",
      vecp_from->nels, vecp_to->nels);
}
