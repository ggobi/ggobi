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

gint
selected_cols_get (gint *cols, datad *d, ggobid *gg)
{
/*
 * Figure out which columns are selected.
*/
  gint j, ncols = 0;

  for (j=0; j<d->ncols; j++) 
    if (d->vartable[j].selected)
      cols[ncols++] = j;

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
          for (k=0; k<display->t1d.nvars; k++)
            cols[ncols++] = display->t1d.vars.els[k];
        break;
         case TOUR2D:
          for (k=0; k<display->t2d.nvars; k++)
            cols[ncols++] = display->t2d.vars.els[k];
        break;
        case COTOUR:
          for (k=0; k<display->tcorr1.nvars; k++)
            cols[ncols++] = display->tcorr1.vars.els[k];
          for (k=0; k<display->tcorr2.nvars; k++)
            cols[ncols++] = display->tcorr2.vars.els[k];
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
    case unknown_display_type:
    break;
  }

  return ncols;
}

/*-------------------------------------------------------------------------*/
/*                         memory management                               */
/*-------------------------------------------------------------------------*/

void
vartable_free_var (gint j, datad *d)
{
  if (d->vartable[j].collab != NULL)
    g_free (d->vartable[j].collab);
  if (d->vartable[j].collab_tform != NULL)
    g_free (d->vartable[j].collab_tform);

  d->vartable[j].collab = NULL;
  d->vartable[j].collab_tform = NULL;
}

static void
vartable_free (datad *d)
{
  gint j;
  for (j=0; j<d->ncols; j++) {
    vartable_free_var (j, d);
  }
  g_free (d->vartable);
  d->vartable = NULL;
}

void
vartable_alloc (datad *d)
{
  if (d->vartable != NULL)
    vartable_free (d);

  d->vartable = (vartabled *) g_malloc (d->ncols * sizeof (vartabled));
}

void
vartable_realloc (gint n, datad *d)
{
  d->vartable = (vartabled *) g_realloc ((gpointer) d->vartable,
    n * sizeof (vartabled));
}

void
vartable_copy_var (gint jfrom, gint jto, datad *d)
{
  g_assert (d->vartable[jfrom].collab != NULL);
  g_assert (d->vartable[jfrom].collab_tform != NULL);

  d->vartable[jto].jref = d->vartable[jfrom].jref;  /*-- jref or jfrom? --*/

  d->vartable[jto].collab = g_strdup (d->vartable[jfrom].collab);
  d->vartable[jto].collab_tform = g_strdup (d->vartable[jfrom].collab_tform);

  d->vartable[jto].mean = d->vartable[jfrom].mean;
  d->vartable[jto].median = d->vartable[jfrom].median;
  d->vartable[jto].lim.min =
    d->vartable[jto].lim_raw.min =
    d->vartable[jto].lim_tform.min = d->vartable[jfrom].lim_tform.min;
  d->vartable[jto].lim.max =
    d->vartable[jto].lim_raw.max =
    d->vartable[jto].lim_tform.max = d->vartable[jfrom].lim_tform.max;

  d->vartable[jto].nmissing = d->vartable[jfrom].nmissing;
  d->vartable[jto].lim_specified_p = d->vartable[jfrom].lim_specified_p;

  /*transform_values_copy (jfrom, jto, d);*/
}


/*-------------------------------------------------------------------------*/

void
vartable_init_var (gint j, datad *d) 
{
  d->vartable[j].selected = false;
  d->vartable[j].nmissing = 0;

  d->vartable[j].categorical_p = false;  /*-- real-valued by default --*/
  d->vartable[j].nlevels = 0;

  d->vartable[j].jref = -1;  /*-- not cloned --*/

  d->vartable[j].mean = 0.0;
  d->vartable[j].median = 0.0;

  d->vartable[j].lim_specified_p = false;  /*-- no user-specified limits --*/

  d->vartable[j].lim_raw.min = 0.0;
  d->vartable[j].lim_raw.max = 0.0;
  d->vartable[j].lim_tform.min = 0.0;
  d->vartable[j].lim_tform.max = 0.0;

  d->vartable[j].tform0 = NO_TFORM0;
  d->vartable[j].domain_incr = 0.;
  d->vartable[j].domain_adj = no_change;
  d->vartable[j].inv_domain_adj = no_change;
  d->vartable[j].tform1 = NO_TFORM1;
  d->vartable[j].param = 0.;
  d->vartable[j].tform2 = NO_TFORM2;

  d->vartable[j].jitter_factor = 0.0;

  d->vartable[j].collab = NULL;
  d->vartable[j].collab_tform = NULL;
}

void vartable_init (datad *d)
{
  gint j;
  for (j=0; j<d->ncols; j++)
    vartable_init_var (j, d);
}

/*-------------------------------------------------------------------------*/
/*                 finding the statistics for the table                    */
/*-------------------------------------------------------------------------*/

void
vartable_stats_print (datad *d, ggobid *gg) 
{
  gint j;
  for (j=0; j<d->ncols; j++) {
    g_printerr ("mean=%f, median=%f\n",
      d->vartable[j].mean, d->vartable[j].median);
    g_printerr ("lims: %7.2f %7.2f %7.2f %7.2f\n",
      d->vartable[j].lim_raw.min, d->vartable[j].lim_raw.max,
      d->vartable[j].lim_tform.min, d->vartable[j].lim_tform.max);
  }
}
