/* scatmat.c */
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

#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <stdlib.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "scatmatClass.h"
#include "scatterplotClass.h"

#define WIDTH 200
#define HEIGHT 200
#define MAXNVARS 4   /* only used to set up the initial matrix */

static const gchar *scatmat_ui =
"<ui>"
"	<menubar>"
"		<menu action='Options'>"
"			<menuitem action='ShowPoints'/>"
"		</menu>"
"	</menubar>"
"</ui>";




displayd *
scatmat_new (displayd *display,
	       gboolean missing_p, gint numRows, gint *rows,
	       gint numCols, gint *cols, datad *d, ggobid *gg) 
{
  GtkWidget *vbox, *frame;
  gint i, j, ctr;
  gint width, height;
  gint scr_width, scr_height;
  gint scatmat_nvars;
  splotd *sp;
  windowDisplayd *wdpy = NULL;
  
  if(!display)
     display = g_object_new(GGOBI_TYPE_SCATMAT_DISPLAY, NULL);

  display_set_values (display, d, gg);
  if(GGOBI_IS_WINDOW_DISPLAY(display))
    wdpy = GGOBI_WINDOW_DISPLAY(display);

  /* If the caller didn't specify the rows and columns, 
     use the default which is the number of variables
     in the dataset or the maximum number of columns
     within a scatterplot matrix.
     ! Need to check rows and cols are allocated. !
   */

  if (numRows == 0 || numCols == 0) {

    scatmat_nvars = MIN (d->ncols, sessionOptions->info->numScatMatrixVars);
    if(scatmat_nvars < 0) {
	scatmat_nvars = d->ncols;
    }

    /* Initialize display with the plotted variables in the current
       display, if appropriate */
    if (gg->current_display != NULL && gg->current_display != display && 
        gg->current_display->d == d && 
        GGOBI_IS_EXTENDED_DISPLAY(gg->current_display))
    {
      gint k, nplotted_vars;
      gint *plotted_vars = (gint *) g_malloc(d->ncols * sizeof(gint));
      displayd *dsp = gg->current_display;

      nplotted_vars = GGOBI_EXTENDED_DISPLAY_GET_CLASS(dsp)->plotted_vars_get(dsp, plotted_vars, d, gg);

      scatmat_nvars = MAX (scatmat_nvars, nplotted_vars);
      for (j=0; j<nplotted_vars; j++)
        rows[j] = cols[j] = plotted_vars[j];
      j = nplotted_vars;
      for (k=0; k<d->ncols; k++) {
        if (!in_vector(k, plotted_vars, nplotted_vars)) {
          rows[j] = cols[j] = k;
          j++;
          if (j == scatmat_nvars)
            break;
        }
      }
      g_free (plotted_vars);

    } else {
      
      for (j=0; j<scatmat_nvars; j++)
        rows[j] = cols[j] = j;
    }

  } else { 
    scatmat_nvars = numRows;
  }

  display->p1d_orientation = HORIZONTAL;
  scatmat_cpanel_init (&display->cpanel, gg);

  /*
   * make the matrix take up no more than some fraction
   * the screen by default, and make the plots square.
  */
  scr_width = gdk_screen_width () / 2;
  scr_height = gdk_screen_height () / 2;
  width = (WIDTH * scatmat_nvars > scr_width) ?
    (scr_width / scatmat_nvars) : WIDTH;
  height = (HEIGHT * scatmat_nvars > scr_height) ?
    (scr_height / scatmat_nvars) : HEIGHT;
  width = height = MIN (width, height);
  /* */
  
  if(wdpy && wdpy->useWindow)
    display_window_init (GGOBI_WINDOW_DISPLAY(display), 
      width*scatmat_nvars, height*scatmat_nvars, 5, gg);

/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 1);
  if(wdpy && wdpy->useWindow) {
    gtk_container_add (GTK_CONTAINER (wdpy->window), vbox);

  display->menu_manager = display_menu_manager_create(display);
  display->menubar = create_menu_bar(display->menu_manager, scatmat_ui, wdpy->window);

  /*
   * After creating the menubar, and populating the file menu,
   * add the Options and Link menus another way
  */
   gtk_box_pack_start (GTK_BOX (vbox), display->menubar, false, true, 0);
  }
/*
 * splots in a table 
*/
  frame = gtk_frame_new (NULL);
  //gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  gtk_box_pack_start (GTK_BOX (vbox), frame, true, true, 1);

  gtk_widget_show (frame);

  display->table = gtk_table_new (scatmat_nvars, scatmat_nvars, false);
  gtk_container_add (GTK_CONTAINER (frame), display->table);
  display->splots = NULL;
  ctr = 0;
  for (i=0; i<scatmat_nvars; i++) {
    for (j=0; j<scatmat_nvars; j++, ctr++) {
/* Can we use SCATTER_SPLOT or do we need SCATMAT_SPLOT. */
      sp = g_object_new(GGOBI_TYPE_SCATMAT_SPLOT, NULL);
      splot_init(sp, display, gg);

      sp->xyvars.x = rows[i]; 
      sp->xyvars.y = cols[j]; 
      sp->p1dvar = (rows[i] == cols[j]) ? cols[j] : -1;

      display->splots = g_list_append (display->splots, (gpointer) sp);

      gtk_table_attach (GTK_TABLE (display->table), sp->da, i, i+1, j, j+1,
			(GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
			(GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
			1, 1);
      gtk_widget_show (sp->da);
    }
  }

  gtk_widget_show (display->table);

  /*-- position the display toward the lower left of the main window --*/
  if(wdpy && wdpy->useWindow) {
    display_set_position (wdpy, gg);
    gtk_widget_show_all (wdpy->window);
  } else {
    gtk_container_add(GTK_CONTAINER(display), vbox);
  }

  return display;
}

/*
 * Find out whether a variable is selected
*/
static gint
scatmat_var_selected (gint jvar, displayd *display)
{
  gint pos = -1;
  GList *l;
  GtkTableChild *child;
  GtkWidget *da;
  splotd *sp;

  for (l=(GTK_TABLE (display->table))->children; l; l=l->next) {
    child = (GtkTableChild *) l->data;
    da = child->widget;
    sp = (splotd *) g_object_get_data(G_OBJECT (da), "splotd");
    if (sp->p1dvar == jvar) {
      pos = child->left_attach;
      break;
    }
  }

  return pos;
}

static splotd *
scatmat_add_plot (gint xvar, gint yvar, gint col, gint row,
		  displayd *display, ggobid *gg)
{
  splotd *sp_new;

  sp_new = g_object_new(GGOBI_TYPE_SCATMAT_SPLOT, NULL);
  splot_init(sp_new, display, gg);

  sp_new->xyvars.x = xvar;
  sp_new->xyvars.y = yvar;
  sp_new->p1dvar = (sp_new->xyvars.x == sp_new->xyvars.y) ? xvar : -1;

  gtk_table_attach (GTK_TABLE (display->table),
		    sp_new->da, col, col+1, row, row+1,
		    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
		    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
		    1, 1);
  gtk_widget_show (sp_new->da);

  /* We don't care where, I think */
  display->splots = g_list_append (display->splots, (gpointer) sp_new);

  return sp_new;
}

gboolean
scatmat_varsel_simple (cpaneld *cpanel, splotd *sp, gint jvar,
  gint *jvar_prev, ggobid *gg)
{
  gboolean redraw = true;
  gboolean Delete = false;
  gint k;
  GList *l;
  splotd *s, *sp_new;
  GtkWidget *da;
  GtkTableChild *child;
  displayd *display = gg->current_display;
  gint jpos, *vars, nvars;
  datad *d = display->d;

  /* Simple:  if jvar is among the plotted variables, delete it;
     otherwise, append it */

  if ((jpos = scatmat_var_selected (jvar, display)) >= 0) {
    l = (GTK_TABLE (display->table))->children;
    while (l) {
      Delete = false;
      child = (GtkTableChild *) l->data;
      l = l->next;
      da = child->widget;

      if (child->left_attach == jpos)
        Delete = true;
      else if (child->left_attach > jpos) {
        child->left_attach--;
        child->right_attach--;
      }
      if (child->top_attach == jpos) {
        Delete = true;
      } else if (child->top_attach > jpos) {
        child->top_attach--;
        child->bottom_attach--;
      }

      if (Delete) {

       s = (splotd *) g_object_get_data(G_OBJECT (da), "splotd");
       display->splots = g_list_remove (display->splots, (gpointer) s);
       /*
        * add a reference to da here, because it's going to be
        * destroyed in splot_free, and we don't want it destroyed
        * as a result of gtk_container_remove.
       */
       gtk_widget_ref (da);
       gtk_container_remove (GTK_CONTAINER (display->table), da);

       if (s == gg->current_splot)
         sp_event_handlers_toggle (s, off, display->cpanel.pmode, 
           display->cpanel.imode);
       splot_free (s, display, gg);
     }
   }

    vars = (gint *) g_malloc(d->ncols * sizeof(gint));
    nvars = GGOBI_EXTENDED_DISPLAY_GET_CLASS(display)->plotted_vars_get(display, vars, d, gg);
    gtk_table_resize (GTK_TABLE (display->table), nvars, nvars);

    /* Make the first plot the current plot */
    gg->current_splot = (splotd *) g_list_nth_data (display->splots, 0);
    display->current_splot = gg->current_splot;
    /* Turn any event handlers back on */
    sp_event_handlers_toggle (gg->current_splot, on, display->cpanel.pmode, 
      display->cpanel.imode);

    redraw = false;  /*-- individual plots don't need a redraw --*/
    g_free(vars);
  } // End of deleting a plot.

/*
 * Otherwise, append a row <and> a column.  When appending a lot,
 * there's no need to change the current splot or to monkey with
 * event handlers.
*/
  else {

    vars = (gint *) g_malloc(d->ncols * sizeof(gint));
    nvars = GGOBI_EXTENDED_DISPLAY_GET_CLASS(display)->plotted_vars_get(display, vars, d, gg);

    /*
     * Now create the new plots and fill in the new row/column.
     * Work out the correct p1dvar/xyvars values for each new plot.
    */

    for (k=0; k<nvars; k++) {
      sp_new = scatmat_add_plot (jvar, vars[k], nvars, k, display, gg);
      if (k != nvars) {  /*-- except at the intersection, do it twice --*/
        sp_new = scatmat_add_plot (vars[k], jvar, k, nvars, display, gg);
      }
    }
    sp_new = scatmat_add_plot (jvar, jvar, nvars, nvars, display, gg);
    /* This isn't the current splot, so I don't want to turn on event
       handlers for it, but the new marginal plot needs to receive drag
       events without ever being the current splot */
    sp_event_handlers_toggle (sp_new, on, DEFAULT_PMODE, DEFAULT_IMODE);


    gtk_table_resize (GTK_TABLE (gg->current_display->table),
                     nvars, nvars);
    redraw = true;
    g_free(vars);
  }

  return redraw;
}

#undef WIDTH
#undef HEIGHT
#undef MAXNVARS
