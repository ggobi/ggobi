/* timeplot.c */
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
 *
 * Contributing author of time series code:  Nicholas Lewin-Koh
*/


#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "tsdisplay.h"

/* initial plot sizes */
#define WIDTH   150
#define HEIGHT  100

/*--------------------------------------------------------------------*/
/*                   Options section                                  */
/*--------------------------------------------------------------------*/

static const gchar* timeplot_ui =
"<ui>"
"	<menubar>"
"		<menu action='Options'>"
"			<menuitem action='ShowPoints'/>"
"			<menuitem action='ShowLines'/>"
"		</menu>"
"	</menubar>"
"</ui>";


void
tsplot_reset_arrangement (displayd *display, gint arrangement, ggobid *gg) 
{
  GList *l;
  GtkWidget *frame, *w;
  splotd *sp;

  if (display->cpanel.tsplot_arrangement == arrangement)
    return;

  for (l=display->splots; l; l=l->next) {
    w = ((splotd *) l->data)->da;
    gtk_widget_ref (w);
    gtk_container_remove (GTK_CONTAINER (gg->tsplot.arrangement_box), w);
  }

  frame = gg->tsplot.arrangement_box->parent;
  gtk_widget_destroy (gg->tsplot.arrangement_box);

/*    if (arrangement == ARRANGE_ROW) */
/*      gg->tsplot.arrangement_box = gtk_hbox_new (true, 0); */
/*    else */
    gg->tsplot.arrangement_box = gtk_vbox_new (true, 0);
  gtk_container_add (GTK_CONTAINER (frame), gg->tsplot.arrangement_box);

  display->p1d_orientation = (arrangement == ARRANGE_ROW) ? VERTICAL :
                                                            HORIZONTAL;
  for (l=display->splots; l; l=l->next) {
    sp = (splotd *) l->data;
    gtk_box_pack_start (GTK_BOX (gg->tsplot.arrangement_box),
                        sp->da, true, true, 0);
    gtk_widget_unref (sp->da);
 }

 /*-- position the display toward the lower left of the main window --*/
  display_set_position (GGOBI_WINDOW_DISPLAY(display), gg);

  gtk_widget_show_all (gg->tsplot.arrangement_box);

  display_tailpipe (display, FULL, gg);

  varpanel_refresh (display, gg);
}


#define MAXNTSPLOTS 6

displayd *
tsplot_new_with_vars (gboolean missing_p, gint nvars, gint *vars, GGobiData *d, ggobid *gg) 
{
  return(tsplot_new(NULL, missing_p, nvars, vars, d, gg));
}

displayd *
tsplot_new(displayd *display, gboolean missing_p, gint nvars, gint *vars, GGobiData *d, ggobid *gg) 
{
  GtkWidget *vbox, *frame;
  gint i, timeVariable, cur;
  splotd *sp;
  gint nplots;

  if(!display)
      display = g_object_new(GGOBI_TYPE_TIME_SERIES_DISPLAY, NULL);

  display_set_values(display, d, gg);

  timeVariable = 0;

  if (nvars == 0) {

      /* See if there is a variable which has been marked as isTime
         and use the first of these as the horizontal axis. */
    for(i = 0; i < d->ncols; i++) {
	vartabled *el;
	el = vartable_element_get(i, d);
	if(el->isTime) {
	    timeVariable = i;
	    break;
	}
    }

    nplots = MIN ((d->ncols-1), sessionOptions->info->numTimePlotVars);
    if (nplots < 0)
      nplots = d->ncols;

    if (gg->current_display != NULL && gg->current_display != display && 
        gg->current_display->d == d && 
        GGOBI_IS_EXTENDED_DISPLAY(gg->current_display))
    {
      gint j, k, nplotted_vars;
      gint *plotted_vars = (gint *) g_malloc(d->ncols * sizeof(gint));
      displayd *dsp = gg->current_display;

      nplotted_vars = GGOBI_EXTENDED_DISPLAY_GET_CLASS(dsp)->plotted_vars_get(dsp, plotted_vars, d, gg);

      nplots = MAX (nplots, nplotted_vars);
      vars[0] = timeVariable;

      /* Loop through plotted_vars.  Don't add timeVariable again, or
	 exceed the number of plots */
      j = 1;
      for (k=0; k<nplotted_vars; k++) {
        if (plotted_vars[k] != timeVariable) {
          vars[j] = plotted_vars[k];
          j++;
          if (j == nplots)
            break;
        }
      }

      /* If we still need more plots, loop through remaining
	 variables.  Don't add timeVariable again, or exceed the
	 number of plots */
      if (j < nplots) {
        for (k=0; k<d->ncols; k++) {
          if (!in_vector(k, plotted_vars, nplotted_vars) && 
              k != timeVariable) 
          {
            vars[j] = k;
            j++;
            if (j == nplots)
              break;
          } 
        }
      }

      g_free (plotted_vars);

    } else {

      for (i=1, cur = 0; i < nplots; i++, cur++) {

	/* Check that we are not setting a variable to the timeVariable
	   but also that we have enough variables. */
	if(cur == timeVariable) {
	    if(cur < d->ncols-1) {
		vars[i] = ++cur;
	    }
	} else
	    vars[i] = cur;
      }
    }


  } else {
    nplots = nvars;
    timeVariable = vars[0];
  }

  tsplot_cpanel_init (&display->cpanel, gg);

  if(GGOBI_WINDOW_DISPLAY(display)->useWindow)
      display_window_init (GGOBI_WINDOW_DISPLAY(display), 
  		2.5*WIDTH, nplots*HEIGHT, 3, gg);

/*
 * Add the main menu bar
*/
  vbox = GTK_WIDGET(display); 
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 1);

  if(GGOBI_WINDOW_DISPLAY(display)->useWindow) {
    gtk_container_add (GTK_CONTAINER (GGOBI_WINDOW_DISPLAY(display)->window), vbox);
	display->menu_manager = display_menu_manager_create(display);
	display->menubar = create_menu_bar(display->menu_manager, timeplot_ui, 
		GGOBI_WINDOW_DISPLAY(display)->window);

    gtk_box_pack_start (GTK_BOX (vbox), display->menubar, false, true, 0);
  }

/*
 * splots in a box in a frame -- either a vbox or an hbox.
*/
  frame = gtk_frame_new (NULL);
  //gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX (vbox), frame, true, true, 1);

/*
 * this is the box that would have to change from horizontal to vertical
 * when the plot arrangement changes
*/
  gg->tsplot.arrangement_box = gtk_vbox_new (true, 0);
  gtk_container_add (GTK_CONTAINER (frame), gg->tsplot.arrangement_box);

  display->splots = NULL;


  for (i=1; i < nplots; i++) {
    sp = ggobi_time_series_splot_new(display, gg);

    sp->xyvars.y = vars[i];
    sp->xyvars.x = timeVariable;

    display->splots = g_list_append (display->splots, (gpointer) sp);
    gtk_box_pack_start (GTK_BOX (gg->tsplot.arrangement_box),
			sp->da, true, true, 0);
  }

  if(GGOBI_WINDOW_DISPLAY(display)->useWindow)
      gtk_widget_show_all (GGOBI_WINDOW_DISPLAY(display)->window);
  else
      gtk_widget_show_all(GTK_WIDGET(gg->tsplot.arrangement_box));

  return display;
}

static gboolean
tsplot_var_selected (gint jvar, displayd *display)
{
  gboolean selected = false;
  splotd *s;
  GList *l = display->splots;
  while (l) {
    s = (splotd *) l->data;
    if (s->xyvars.y == jvar || s->xyvars.x == jvar ) {
      selected = true;
      break;
    }
    l = l->next ;
  }
  return selected;
}

gboolean
tsplot_varsel (GtkWidget *w, displayd *display, splotd *sp, gint jvar,
    gint toggle, gint mouse, cpaneld *cpanel,  ggobid *gg)
{
  gboolean redraw = true;
  gint nplots = g_list_length (gg->current_display->splots);
  gint k;
  gint jvar_indx=-1, new_indx;
  GList *l;
  splotd *s, *sp_new;
  GtkWidget *box;

  /*
   *  if left button click, the x variable no matter what
   *  selection_mode prevails.
  */
  if (toggle == VARSEL_X || mouse == 1) {
    l = display->splots;
    s = (splotd *) l->data;
    if (s->xyvars.x == jvar) redraw=false;  /*-- this is already the x var --*/
    else {
      while (l) {
        s = (splotd *) l->data;
        s->xyvars.x = jvar;
        l = l->next;
      }
    }

  } else if (toggle == VARSEL_Y || mouse == 2 || mouse == 3) {

    if (tsplot_var_selected (jvar, display)) {  /* then delete */

      /* If jvar is one of the plotted variables, its corresponding plot */
      splotd *jvar_sp = NULL;

      k = 0;
      l = display->splots;
      while (l) {
        s = (splotd *) l->data;
        if (s->xyvars.y == jvar) {
          jvar_sp = s;
          jvar_indx = k;
          break;
        }
        l = l->next;
        k++;
      }

      if (jvar_sp != NULL && nplots > 1) {
        /*-- Delete the plot from the list, and destroy it. --*/
        display->splots = g_list_remove (display->splots, (gpointer) jvar_sp);
        nplots--;

        /*
         * If the plot being removed is the current plot, reset
         * gg->current_splot.
         */
        if (jvar_sp == gg->current_splot) {
          sp_event_handlers_toggle (sp, off, cpanel->pmode, cpanel->imode);

          new_indx = (jvar_indx == 0) ? 0 : MIN (nplots-1, jvar_indx);
          gg->current_splot = (splotd *)
            g_list_nth_data (display->splots, new_indx);
          /* just for insurance, to handle the unforeseen */
          if (gg->current_splot == NULL) 
            gg->current_splot = (splotd *) g_list_nth_data (display->splots, 0);
          display->current_splot = gg->current_splot;
          sp_event_handlers_toggle (gg->current_splot, on, cpanel->pmode, cpanel->imode);
        }

        splot_free (jvar_sp, display, gg);
      }

    } else {  /* Append */

      l = display->splots;
      s = (splotd *) l->data; /* this let us set the x var for the new plot
                                 to be the same as that of the first plot. */
      sp_new = ggobi_time_series_splot_new (display, gg);
      sp_new->xyvars.y = jvar;
      sp_new->xyvars.x = s->xyvars.x;

      display->splots = g_list_append (display->splots,
        (gpointer) sp_new);

      box = (sp->da)->parent;
      gtk_box_pack_end (GTK_BOX (box), sp_new->da, true, true, 0);
      gtk_widget_show (sp_new->da);
      //gg->current_splot = sp->displayptr->current_splot = sp_new;

    /* I don't think it's possible to initialize brushing until the
       data has run through the pipeline.  Since I can't cleanly add a
       plot in brushing mode, I think it's best to switch back to the
       default mode.  -- dfs
       */
      GGOBI(full_viewmode_set)(EXTENDED_DISPLAY_PMODE, DEFAULT_IMODE, gg);

      /* Initialize drag and drop for the new panel */
      sp_event_handlers_toggle (sp_new, on, cpanel->pmode, cpanel->imode);
      redraw = true;
    }
  }

  return redraw;
}

/*--------------------------------------------------------------------*/
/*               The whiskers for timeseries lines                    */  
/*--------------------------------------------------------------------*/

static void
tsplot_rewhisker (splotd *sp, ggobid *gg) 
{
  gint i, k, n;
  displayd *display = (displayd *) sp->displayptr;
  GGobiData *d = display->d;
  gboolean draw_whisker;

  if (d->nmissing) {
    g_assert (d->missing.nrows == d->nrows);
    g_assert (d->missing.ncols == d->ncols);
  }

  for (k=0; k<(d->nrows_in_plot-1); k++) {
    i = d->rows_in_plot.els[k];
    n = d->rows_in_plot.els[k+1];
    
    /*-- .. also if we're not drawing missings, and an endpoint is missing --*/
    if (d->nmissing > 0 && !d->missings_show_p &&
          (d->missing.vals[i][sp->xyvars.x] || 
           d->missing.vals[i][sp->xyvars.y] ||
           d->missing.vals[n][sp->xyvars.x] || 
           d->missing.vals[n][sp->xyvars.y]) &&
           (sp->screen[i].x > sp->screen[n].x))/* to keep time going
                                                  forwards */
    {
      draw_whisker = false;
    }
    else
      draw_whisker = true;
    /* --- all whiskers --- */
    if (draw_whisker) {
      sp->whiskers[i].x1 = sp->screen[i].x;
      sp->whiskers[i].y1 = sp->screen[i].y;
      sp->whiskers[i].x2 = sp->screen[n].x;
      sp->whiskers[i].y2 = sp->screen[n].y;
    }      
  }
}


/*-- set the positions of the whiskers for sp and prev_sp --*/
void
tsplot_whiskers_make (splotd *sp, displayd *display, ggobid *gg) {
  GList *splist;
  splotd *splot;
  splotd *sp_next = (splotd *) NULL;

  for (splist = display->splots; splist; splist = splist->next) {
    splot = (splotd *) splist->data;
    if (splot == sp) {
/*-- interesting -- what's sp_next used for?   dfs --*/
      sp_next = (splist->next == NULL) ? NULL : (splotd *) splist->next->data;
    }
  }

  tsplot_rewhisker (sp, gg);
}
