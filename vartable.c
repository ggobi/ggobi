/* vartable.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
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
  PipelineMode mode = viewmode_get (gg);
  gint ncols = 0;
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;
  gint k;

  switch (display->displaytype) {
    case scatterplot:
      switch (mode) {
        case P1PLOT:
          cols[ncols++] = sp->p1dvar;
        break;
        case XYPLOT:
          cols[ncols++] = sp->xyvars.x;
          cols[ncols++] = sp->xyvars.y;
        break;
        case TOUR1D:
          for (k=0; k<display->t1d.nactive; k++)
            cols[ncols++] = display->t1d.active_vars.els[k];
        break;
         case TOUR2D:
          for (k=0; k<display->t2d.nactive; k++)
            cols[ncols++] = display->t2d.active_vars.els[k];
        break;
        case COTOUR:
          for (k=0; k<display->tcorr1.nactive; k++)
            cols[ncols++] = display->tcorr1.active_vars.els[k];
          for (k=0; k<display->tcorr2.nactive; k++)
            cols[ncols++] = display->tcorr2.active_vars.els[k];
        break;

        case NULLMODE:
        case ROTATE:
        case SCALE:
        case BRUSH:
        case IDENT:
        case EDGEED:
        case MOVEPTS:
        case SCATMAT:
        case PCPLOT:
        case TSPLOT:
#ifdef BARCHART_IMPLEMENTED
            case BARCHART:
#endif
        case NMODES:
        break;
      }
    break;
    case scatmat:
    {
      GList *l;
      splotd *s;
      for (l=display->splots; l; l=l->next) {
        s = (splotd *) l->data;
        if (s->p1dvar == -1) {
          if (!array_contains (cols, ncols, s->xyvars.x))
            cols[ncols++] = s->xyvars.x;
          if (!array_contains (cols, ncols, s->xyvars.y))
            cols[ncols++] = s->xyvars.y;
        } else {
          if (!array_contains (cols, ncols, s->p1dvar))
            cols[ncols++] = s->p1dvar;
        }
      }
    }
    break;
    case parcoords:
    {
      GList *l;
      splotd *s;
      for (l=display->splots; l; l=l->next) {
        s = (splotd *) l->data;
        if (!array_contains (cols, ncols, s->p1dvar))
          cols[ncols++] = s->p1dvar;
      }
    }
    break;
    case tsplot:
    {
      GList *l;
      splotd *s;
      for (l=display->splots; l; l=l->next) {
        s = (splotd *) l->data;
        if (!array_contains (cols, ncols, s->xyvars.y))
          cols[ncols++] = s->xyvars.y;
      }
    }
    break;
#ifdef BARCHART_IMPLEMENTED
    case barchart:
      cols[ncols++] = sp->p1dvar;
    break;
#endif

    case unknown_display_type:
    break;
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
vartable_alloc (datad *d)
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

  vt_to->categorical_p = vt_from->categorical_p;
  vt_to->nlevels = vt_from->nlevels;
  vt_to->level_values = NULL;
  vt_to->level_names = g_array_new (false, false, sizeof (gchar *));
  for (k=0; k<vt_to->nlevels; k++) {
    vt_to->level_values = g_list_append (vt_to->level_values,
      g_list_nth_data (vt_from->level_values, k));
    g_array_append_val (vt_to->level_names, 
      g_array_index (vt_from->level_names, gchar *, k));
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

  vt->selected = false;
  vt->nmissing = 0;

  vt->categorical_p = false;  /*-- real-valued by default --*/
  vt->nlevels = 0;

  vt->jref = -1;  /*-- not cloned --*/

  vt->mean = 0.0;
  vt->median = 0.0;

  vt->lim_specified_p = false;  /*-- no user-specified limits --*/

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
