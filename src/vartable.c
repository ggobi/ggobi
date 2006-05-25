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

#include "ggobi-variable.h"

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
  GGobiVariable *var;

  for (j=0; j<GGOBI_STAGE(d)->n_cols; j++) {
    var = ggobi_stage_get_variable(GGOBI_STAGE(d), j);
    if (var->selected)
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
