/* vartable.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering intoa license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
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

vartabled *
vartable_element_get (gint j, datad *d)
{
  return ((vartabled *) g_slist_nth_data (d->vartable, j));
}
gint
vartable_index_get_by_name (gchar *collab, datad *d)
{
  gint j = -1;
  vartabled *vt;

  for (j=0; j<g_slist_length (d->vartable); j++) {
    vt = (vartabled *) g_slist_nth_data (d->vartable, j);
    if (strcmp (vt->collab, collab) == 0) {
      break;
    }
  }

  return j;
}
vartabled *
vartable_element_get_by_name (gchar *collab, datad *d)
{
  gint j;
  vartabled *vt;

  for (j=0; j<g_slist_length (d->vartable); j++) {
    vt = (vartabled *) g_slist_nth_data (d->vartable, j);
    if (strcmp (vt->collab, collab) == 0) {
      return (vt);
    }
  }

  return ((vartabled *) NULL);
}
void
vartable_element_append (vartabled *vt, datad *d)
{
  d->vartable = g_slist_append (d->vartable, vt);
}
void
vartable_element_remove (gint j, datad *d)
{
  vartabled *vt = vartable_element_get (j, d);
  d->vartable = g_slist_remove (d->vartable, vt);
}

gint
selected_cols_get (gint *cols, datad *d, ggobid *gg)
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
plotted_cols_get (gint *cols, datad *d, ggobid *gg) 
{
  gint ncols = 0;
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;

  if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
     GtkGGobiExtendedDisplayClass *klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
     ncols = klass->plotted_vars_get(display, cols, d, gg);
  }

  return ncols;
}

/*-------------------------------------------------------------------------*/
/*                         memory management                               */
/*-------------------------------------------------------------------------*/

void
vartable_free_element (gint j, datad *d)
{
  vartabled *vt = vartable_element_get (j, d); 

  if (vt->collab != NULL)
    g_free (vt->collab);
  if (vt->collab_tform != NULL)
    g_free (vt->collab_tform);

  vartable_element_remove (j, d);
}

void
vartable_free (datad *d)
{
  gint j;
  for (j=d->ncols-1; j >= 0; j--) {
    vartable_free_element (j, d);
  }
  g_slist_free (d->vartable);
  d->vartable = NULL;
}

void
vartable_alloc (datad *d)  /* weird -- nothing is allocated here --*/
{
  if (d->vartable != NULL)
    vartable_free (d);

  d->vartable = NULL;
}

void
vartable_copy_var (gint jfrom, gint jto, datad *d)
{
  gint k;
  vartabled *vt_from = vartable_element_get (jfrom, d);
  vartabled *vt_to = vartable_element_get (jto, d);

  g_assert (vt_from->collab != NULL);
  g_assert (vt_from->collab_tform != NULL);

  vt_to->jref = vt_from->jref;  /*-- jref or jfrom? --*/

  vt_to->collab = g_strdup (vt_from->collab);
  vt_to->collab_tform = g_strdup (vt_from->collab_tform);

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

  vt_to->nmissing = vt_from->nmissing;
  vt_to->lim_specified_p = vt_from->lim_specified_p;
}


/*-------------------------------------------------------------------------*/

vartabled *
vartable_element_new (datad *d) 
{
  vartabled *vt = (vartabled *) g_malloc (sizeof (vartabled));

  vt->d = (struct datad *) d; /* the compiler insists */

  vt->selected = false;
  vt->nmissing = 0;

  vt->vartype = real;  /*-- real-valued by default --*/
  vt->nlevels = 0;

  vt->jref = -1;  /*-- not cloned --*/

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

void vartable_init (datad *d)
{
  gint j;
  for (j=0; j<d->ncols; j++)
    vartable_element_new (d);
}

/*-------------------------------------------------------------------------*/
/*                 finding the statistics for the table                    */
/*-------------------------------------------------------------------------*/

void
vartable_stats_print (datad *d, ggobid *gg) 
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
