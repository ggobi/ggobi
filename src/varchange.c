/* varchange.c: add or delete variables, including cloning */
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
#include "utils_ui.h"

#include "ggobi-variable.h"


// displays should eventually listen to pipeline themselves
void
tour_realloc_up (GGobiStage *d, gint nc)
{
  displayd *dsp;
  GList *l;

  if (!GGOBI_IS_SESSION(d->gg))
    return;

  for (l=d->gg->displays; l; l=l->next) {
    GGobiExtendedDisplayClass *klass;
    dsp = (displayd *) l->data;

    if(!GGOBI_IS_EXTENDED_DISPLAY(dsp))
      continue;
    klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS(dsp);
    if(klass->tourcorr_realloc)
        klass->tourcorr_realloc(dsp, nc, d);
    if(klass->tour2d3_realloc)
        klass->tour2d3_realloc(dsp, nc, d);
    if(klass->tour2d_realloc)
        klass->tour2d_realloc(dsp, nc, d);
    if(klass->tour1d_realloc)
        klass->tour1d_realloc(dsp, nc, d);
  }
}

void
clone_vars (gint * cols, gint ncols, GGobiStage * d)
{
  gint i, k, jfrom, jto;
  GGobiStage *root = ggobi_stage_get_root(d);
  gint nprev = ggobi_data_add_cols(GGOBI_DATA(root), ncols);
  
  for (k = 0; k < ncols; k++) {
    jfrom = cols[k];        
    jto = nprev + k; 

    /*-- copy the data --*/
    for (i = 0; i < d->n_rows; i++) {
      ggobi_stage_set_raw_value(d, i, jto, ggobi_stage_get_raw_value(d, i, jfrom));
      if (ggobi_stage_is_missing(d, i, jfrom))
        ggobi_stage_set_missing(d, i, jto);
    }

    /*clone = ggobi_variable_clone(ggobi_stage_get_variable(d, jfrom));
    ggobi_stage_set_variable(d, jto, clone);*/
    ggobi_stage_set_col_name(d, jto, ggobi_stage_get_col_name(d, jfrom));
    ggobi_stage_update_col(d, (guint) jto);
  }
  ggobi_stage_flush_changes(d);
}


static gint
is_variable_plotted (GSList *cols, GGobiStage * d)
{
  GList *dlist;
  displayd *display;
  gint jplotted = -1;

  if (!GGOBI_IS_SESSION(d->gg))
    return 0;

  /*-- check each display for each variable --*/
  for (dlist = d->gg->displays; dlist; dlist = dlist->next) {
    display = (displayd *) dlist->data;
    if (display->d != d)
      continue;

    if (jplotted >= 0)
      break;

    if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
      GGobiExtendedDisplayClass *klass;
      klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
      jplotted = klass->variable_plotted_p (display, cols, d);
    }
  }

  return jplotted;
}

gboolean
delete_vars (gint *cols_arr, gint ncols, GGobiStage * d)
{
  gint j;
  GSList *cols = NULL;
  d = ggobi_stage_get_root(d);
  
  /*-- don't allow all variables to be deleted --*/
  g_return_val_if_fail(ncols < d->n_cols, d->n_cols);
  
  for (j = 0; j < ncols; j++)
    cols = g_slist_prepend(cols, GINT_TO_POINTER(cols_arr[j]));
  cols = g_slist_reverse(cols);
  
  /*
   * If one of the variables to be deleted is currently plotted,
   * we won't proceed until the user cleans up.
   */
   // FIXME: If we are to get rid of delete_vars(), we need to allow specification 
   // of a 'hook' function that determines whether deleting a variable is valid
   // given the current state.
  if ((j = is_variable_plotted (cols, d)) != -1) {
    gchar *message;
    message =
      g_strdup_printf
      ("Deletion failed; the variable '%s' is currently plotted\n",
       ggobi_stage_get_col_name(d, j));
    quick_message (message, false);
    g_free (message);

    return false;
  }
  
  ggobi_data_delete_cols(GGOBI_DATA(d), cols);
  
  g_slist_free(cols);
  return true;
}
