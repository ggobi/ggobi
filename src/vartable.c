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

gint
selected_cols_get (gint *cols, GGobiData *d, ggobid *gg)
{
/*
 * Figure out which columns are selected.
*/
  gint j, ncols = 0;
  vartabled *vt;

  for (j=0; j<d->ncols; j++) {
    vt = ggobi_data_get_vartable(d, j);
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

vartabled *
vartable_copy_var (vartabled *vt, vartabled *vt_to)
{
  vt_to->collab = g_strdup (vt->collab);
  vt_to->collab_tform = g_strdup (vt->collab_tform);
  vt_to->nickname = g_strdup (vt->nickname);

  vt_to->vartype = vt->vartype;
  vt_to->nlevels = vt->nlevels;
  if (vt->nlevels && vt->vartype == categorical) {
    vt_to->level_values = (gint*)
      g_malloc(sizeof(gint) * vt->nlevels);
    vt_to->level_counts = (gint*)
      g_malloc(sizeof(gint) * vt->nlevels);
    vt_to->level_names =  (gchar **)
      g_malloc(sizeof(gchar *) * vt->nlevels);
  } else {
    vt_to->level_values = NULL;
    vt_to->level_counts = NULL;
    vt_to->level_names = NULL;
  }
  for (gint k=0; k<vt_to->nlevels; k++) {
    vt_to->level_values[k] = vt->level_values[k];
    vt_to->level_counts[k] = vt->level_counts[k];
    vt_to->level_names[k] = g_strdup(vt->level_names[k]);
  }

  vt_to->mean = vt->mean;
  vt_to->median = vt->median;
  vt_to->lim.min =
    vt_to->lim_raw.min =
    vt_to->lim_tform.min = vt->lim_tform.min;
  vt_to->lim.max =
    vt_to->lim_raw.max =
    vt_to->lim_tform.max = vt->lim_tform.max;

  vt_to->lim_display.min = vt->lim_display.min;
  vt_to->lim_display.max = vt->lim_display.max;

  vt_to->lim_specified_p = vt->lim_specified_p;
  
  vt_to->tform0 = vt->tform0;
  vt_to->tform1 = vt->tform1;
  vt_to->tform2 = vt->tform2;
  vt_to->domain_incr = vt->domain_incr;
  vt_to->param = vt->param;
  vt_to->domain_adj = vt->domain_adj;
  vt_to->inv_domain_adj = vt->inv_domain_adj;
  
  return vt_to;
}


/*-------------------------------------------------------------------------*/

vartabled *
vartable_element_new (GGobiData *d) 
{
  vartabled *vt = (vartabled *) g_malloc (sizeof (vartabled));

  if (d) 
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

  return vt;
}
