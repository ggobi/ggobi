/* vartable.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "vartable.h"

#ifdef __cplusplus
extern "C" {
#endif
extern gfloat no_change (gfloat, gfloat);
#ifdef __cplusplus
}
#endif


/*-------------------------------------------------------------------------*/
/*                         utilities                                       */
/*-------------------------------------------------------------------------*/

/*-- also used in varchange.c --*/
gboolean
array_contains (gint* arr, gint n, gint el)
{
  gint j;

  for (j=0; j<n; j++)
    if (arr[j] == el)
      return true;
  
  return false;
}

//FIXME: to become: ggobi_data_get_vartable(self, guint j) ?
vartabled *
vartable_element_get (gint j, GGobiData *d)
{
  vartabled *vt = (vartabled *) NULL;

  if (j < 0 || j >= d->ncols)
    g_printerr ("(vartable_element_get) illegal variable number %d\n", j);
  else
    vt = (vartabled *) g_slist_nth_data (d->vartable, j);

  return (vt);
}
gint
vartable_index_get_by_name(gchar *name, GGobiData *d)
{
  gint j;
  gint index = -1;
  vartabled *vt;
  for (j=0; j<d->ncols; j++) {
    vt = vartable_element_get(j, d);
    if (strcmp(vt->collab, name) == 0) {
      index = j;
      break;
    }
  }
  return index;
}

void
vartable_element_append (vartabled *vt, GGobiData *d)
{
  d->vartable = g_slist_append (d->vartable, vt);
}
void
vartable_element_remove (gint j, GGobiData *d)
{
  vartabled *vt = vartable_element_get (j, d);
  d->vartable = g_slist_remove (d->vartable, vt);
}

gint
selected_cols_get (gint *cols, GGobiData *d, ggobid *gg)
{
/*
 * Figure out which columns are selected.
*/
  gint j, ncols = 0;
  vartabled *vt;

  for (j=0; j<d->ncols; j++) {
    vt = vartable_element_get (j, d);
    if (vt->selected)
      cols[ncols++] = j;
  }

  return (ncols);
}

/*
 * When there aren't any columns in the variable statistics table,
 * this is how we find out which columns are selected for plotting.
*/
gint
plotted_cols_get (gint *cols, GGobiData *d, ggobid *gg) 
{
  gint ncols = 0;
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;

  if(GGOBI_IS_EXTENDED_DISPLAY(display)) {
     GGobiExtendedDisplayClass *klass;
     klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS(display);
     ncols = klass->plotted_vars_get(display, cols, d, gg);
  }

  return ncols;
}


/*-------------------------------------------------------------------------*/
/*                         memory management                               */
/*-------------------------------------------------------------------------*/

void
vartable_free_element (gint j, GGobiData *d)
{
  vartabled *vt = vartable_element_get (j, d); 

  if (vt->collab != NULL)
    g_object_unref (vt->collab);
  if (vt->collab_tform != NULL)
    g_object_unref (vt->collab_tform);

  vartable_element_remove (j, d);
}

void
vartable_free (GGobiData *d)
{
  gint j;
  for (j=d->ncols-1; j >= 0; j--) {
    vartable_free_element (j, d);
  }
  g_slist_free (d->vartable);
  d->vartable = NULL;
}

void
vartable_alloc (GGobiData *d)  /* weird -- nothing is allocated here --*/
{
  if (d->vartable != NULL)
    vartable_free (d);

  d->vartable = NULL;
}


//FIXME: should be removed and replaced by vartable_clone
//also see code in missings.c
void
vartable_copy_var (gint jfrom, gint jto, GGobiData *d)
{
  gint k;
  vartabled *vt_from = vartable_element_get (jfrom, d);
  vartabled *vt_to = vartable_element_get (jto, d);

  g_assert (vt_from->collab != NULL);
  g_assert (vt_from->collab_tform != NULL);

  vt_to->collab = g_strdup (vt_from->collab);
  vt_to->collab_tform = g_strdup (vt_from->collab_tform);
  vt_to->nickname = g_strdup (vt_from->nickname);

  vt_to->vartype = vt_from->vartype;
  vt_to->nlevels = vt_from->nlevels;
  if (vt_from->nlevels && vt_from->vartype == categorical) {
    vt_to->level_values = (gint*)
      g_malloc(sizeof(gint) * vt_from->nlevels);
    vt_to->level_counts = (gint*)
      g_malloc(sizeof(gint) * vt_from->nlevels);
    vt_to->level_names =  (gchar **)
      g_malloc(sizeof(gchar *) * vt_from->nlevels);
  } else {
    vt_to->level_values = NULL;
    vt_to->level_counts = NULL;
    vt_to->level_names = NULL;
  }
  for (k=0; k<vt_to->nlevels; k++) {
    vt_to->level_values[k] = vt_from->level_values[k];
    vt_to->level_counts[k] = vt_from->level_counts[k];
    vt_to->level_names[k] = g_strdup(vt_from->level_names[k]);
  }

  vt_to->mean = vt_from->mean;
  vt_to->median = vt_from->median;
  vt_to->lim.min =
    vt_to->lim_raw.min =
    vt_to->lim_tform.min = vt_from->lim_tform.min;
  vt_to->lim.max =
    vt_to->lim_raw.max =
    vt_to->lim_tform.max = vt_from->lim_tform.max;

  vt_to->lim_display.min = vt_from->lim_display.min;
  vt_to->lim_display.max = vt_from->lim_display.max;

  vt_to->lim_specified_p = vt_from->lim_specified_p;
}

/*-------------------------------------------------------------------------*/

vartabled *
vartable_element_new (GGobiData *d) 
{
  vartabled *vt = (vartabled *) g_malloc (sizeof (vartabled));

  vt->d = G_OBJECT(d);

  vt->selected = false;

  vt->vartype = real;  /*-- real-valued by default --*/
  vt->nlevels = 0;

  vt->mean = 0.0;
  vt->median = 0.0;

  vt->lim_specified_p = false;  /*-- no user-specified limits --*/
  vt->lim_specified.min = 0.0;
  vt->lim_specified.max = 0.0;
  vt->lim_specified_tform.min = 0.0;
  vt->lim_specified_tform.max = 0.0;

  vt->lim_raw.min = 0.0;
  vt->lim_raw.max = 0.0;
  vt->lim_tform.min = 0.0;
  vt->lim_tform.max = 0.0;

  vt->tform0 = NO_TFORM0;
  vt->domain_incr = 0.;
  vt->domain_adj = no_change;
  vt->inv_domain_adj = no_change;
  vt->tform1 = NO_TFORM1;
  vt->param = 0.;
  vt->tform2 = NO_TFORM2;

  vt->jitter_factor = 0.0;

  vt->collab = NULL;
  vt->collab_tform = NULL;

  vartable_element_append (vt, d);
  return vt;
}

void vartable_init (GGobiData *d)
{
  gint j;
  for (j=0; j<d->ncols; j++)
    vartable_element_new (d);
}

void
vartable_element_categorical_init(vartabled *vt,
  gint nlevels, gchar **level_names, gint *level_values, gint *level_counts)
{
  gint i;
  if(vt) {
    vt->vartype = categorical;
    vt->nlevels = nlevels;
    vt->level_names = (gchar **) g_malloc(sizeof(gchar*) * nlevels);
    vt->level_values = (gint *) g_malloc(sizeof(gint) * nlevels);
    vt->level_counts = (gint *) g_malloc(sizeof(gint) * nlevels);
    for(i = 0 ; i < nlevels; i++) {
      vt->level_names[i] = g_strdup(level_names[i]);
      if (level_counts)
        vt->level_counts[i] = level_counts[i];
	    else vt->level_counts[i] = 0;
      if (level_values)
   	    vt->level_values[i] = level_values[i];
	    else vt->level_values[i] = i+1;
   }
  }
}

/*-------------------------------------------------------------------------*/
/*                 finding the statistics for the table                    */
/*-------------------------------------------------------------------------*/

void
vartable_stats_print (GGobiData *d, ggobid *gg) 
{
  gint j;
  vartabled *vt;

  for (j=0; j<d->ncols; j++) {
    vt = vartable_element_get (j, d);
    g_printerr ("mean=%f, median=%f\n", vt->mean, vt->median);
    g_printerr ("lims: %7.2f %7.2f %7.2f %7.2f\n",
      vt->lim_raw.min, vt->lim_raw.max,
      vt->lim_tform.min, vt->lim_tform.max);
  }
}

gint
checkLevelValue (vartabled *vt, double value)
{
  gint i;
  for (i = 0; i < vt->nlevels; i++) {
    if (vt->level_values[i] == (int) value)
      return (i);
  }

  return (-1);
}
