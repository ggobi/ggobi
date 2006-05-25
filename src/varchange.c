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

#include "ggobi-variable.h"


// displays should eventually listen to pipeline themselves
void
tour_realloc_up (GGobiData *d, gint nc)
{
  displayd *dsp;
  GList *l;

  if (!GGOBI_IS_GGOBI(d->gg))
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


guint
create_explicit_variable (GGobiData * d, gchar * vname, NewVariableType vartype)
{
  guint jvar = ggobi_data_add_cols(d, 1);
  ggobi_stage_set_col_name(GGOBI_STAGE(d), jvar, vname);

  for (guint i = 0; i < GGOBI_STAGE(d)->n_rows; i++) {
    switch(vartype) {
      case ADDVAR_ROWNOS:
        ggobi_stage_set_raw_value(GGOBI_STAGE(d), i, jvar, (gfloat) (i + 1));
        break;
      case ADDVAR_BGROUP:
        ggobi_stage_set_raw_value(GGOBI_STAGE(d), i, jvar, (gfloat) d->clusterid.els[i]);
        break;
    }
  }
  g_signal_emit_by_name(d, "col_data_changed", jvar);
  return jvar;
}

void
clone_vars (gint * cols, gint ncols, GGobiData * d)
{
  gint i, k, jfrom, jto;
  gint nprev = ggobi_data_add_cols(d, ncols);
  
  for (k = 0; k < ncols; k++) {
    GGobiVariable *clone;
    
    jfrom = cols[k];        
    jto = nprev + k; 

    /*-- copy the data --*/
    for (i = 0; i < GGOBI_STAGE(d)->n_rows; i++) {
      ggobi_stage_set_raw_value(GGOBI_STAGE(d), i, jto, d->tform.vals[i][jfrom]);
      if (ggobi_stage_is_missing(GGOBI_STAGE(d), i, jfrom))
        ggobi_stage_set_missing(GGOBI_STAGE(d), i, jto);
    }

    clone = ggobi_variable_clone(ggobi_stage_get_variable(GGOBI_STAGE(d), jfrom));
    ggobi_data_set_variable(d, jto, clone);
    g_signal_emit_by_name(d, "col_data_changed", (guint) jto);
  }
}


static gint
is_variable_plotted (gint * cols, gint ncols, GGobiData * d)
{
  GList *dlist;
  displayd *display;
  gint jplotted = -1;

  if (!GGOBI_IS_GGOBI(d->gg))
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
      jplotted = klass->variable_plotted_p (display, cols, ncols, d);
    }
  }

  return jplotted;
}

gboolean
delete_vars (gint * cols, gint ncols, GGobiData * d)
{
  gint j;
  guint *ucols;
  
  /*-- don't allow all variables to be deleted --*/
  g_return_val_if_fail(ncols < GGOBI_STAGE(d)->n_cols, GGOBI_STAGE(d)->n_cols);
  
  /*
   * If one of the variables to be deleted is currently plotted,
   * we won't proceed until the user cleans up.
   */
   // FIXME: If we are to get rid of delete_vars(), we need to allow specification 
   // of a 'hook' function that determines whether deleting a variable is valid
   // given the current state.
  if ((j = is_variable_plotted (cols, ncols, d)) != -1) {
    gchar *message;
    message =
      g_strdup_printf
      ("Deletion failed; the variable '%s' is currently plotted\n",
       ggobi_stage_get_col_name(GGOBI_STAGE(d), j));
    quick_message (message, false);
    g_free (message);

    return false;
  }

  // FIXME: Need to move everything to unsigned ints
  ucols = g_new(guint, ncols);
  for (j = 0; j < ncols; j++)
    ucols[j] = (guint)cols[j];
  
  ggobi_data_delete_cols(d, ucols, ncols);

  g_free(ucols);
  
  return true;
}
