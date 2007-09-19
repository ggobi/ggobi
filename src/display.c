/* display.c */
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

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include <string.h>
#include "vars.h"
#include "externs.h"

#include "display_tree.h"

DisplayOptions DefaultDisplayOptions = {
  true,                         /* points_show_p */
  false,                         /* axes_show_p */
  true,                         /* axes_label_p */
  false,                        /* axes_values_p */
  false,                        /* edges_undirected_show_p */
  false,                        /* edges_arrowheads_show_p */
  false,                        /* edges_directed_show_p */
  true,                         /* whiskers_show_p */
/* unused
                                         true,  * missings_show_p  * 
                                         true,  * axes_center_p *
                                         true,  * double_buffer_p *
                                         true   * link_p *
*/
};


/*-- debugging utility --*/
/*
static void
displays_print (GGobiSession *gg) {
  GList *l;
  displayd *dsp;

  g_printerr ("n displays = %d\n", g_list_length (gg->displays));
  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
    if (dsp != NULL)
      g_printerr ("  display %d (type=%d)\n", (gint) dsp, dsp->displaytype);
    else
      g_printerr ("  dsp = NULL!\n");
  }
}
*/

/*----------------------------------------------------------------------*/
/*               Drawing routines                                       */
/*----------------------------------------------------------------------*/

/*-- replot all splots in display --*/
/*-- type = EXPOSE, QUICK, BINNED, FULL --*/
void
display_plot (displayd * display, RedrawStyle type, GGobiSession * gg)
{
  GList *slist;
  splotd *sp;

  for (slist = display->splots; slist; slist = slist->next) {
    sp = (splotd *) slist->data;
    if (sp != NULL) {
      splot_redraw (sp, type, gg);
    }
  }
}

static void
display_plot_allbutone (displayd * display, splotd * splot,
                        RedrawStyle type, GGobiSession * gg)
{
  GList *slist;
  splotd *sp;

  for (slist = display->splots; slist; slist = slist->next) {
    sp = (splotd *) slist->data;
    if (sp == NULL);
    else if (splot == NULL || sp != splot)
      splot_redraw (sp, type, gg);
  }
}

/*----------------------------------------------------------------------*/
/*           Callbacks common to multiple display types                 */
/*----------------------------------------------------------------------*/

/*
We need to allow people to programmatically change a setting and force
the update.  The current framework is all based on GUI events and so
we don't update the GUI components here as we assume they are set
appropriately.  A Model View Controller approach is needed.
*/
void
set_display_option (gboolean active, guint action, displayd * display)
{
  GGobiSession *gg = display->ggobi;
  gchar *title;
  gint ne = 0;
  GGobiStage *onlye = NULL;

  g_return_if_fail (GGOBI_IS_DISPLAY (display));

  /*-- count edge sets --*/
  if (action == DOPT_EDGES_U || action == DOPT_EDGES_D ||
      action == DOPT_EDGES_A || action == DOPT_EDGES_H) {
    gint k, nd = g_slist_length (gg->d);
    GGobiStage *e;
    for (k = 0; k < nd; k++) {
      e = (GGobiStage *) g_slist_nth_data (gg->d, k);
      if (ggobi_stage_get_n_edges(e)) {
        ne++;
        onlye = e;            /* meaningful if there's only one */
      }
    }
    if (ne != 1)
      onlye = NULL;
  }
  /*--  --*/

  switch (action) {
  case DOPT_POINTS:
    display->options.points_show_p = active;
    display_plot (display, FULL, gg);
    break;

  case DOPT_EDGES_U:  /*-- undirected: edges only --*/
    display->options.edges_undirected_show_p = active;
    /*if (active) {
       display_edges_directed_show (display, false);
       display_edges_arrowheads_show (display, false);
       } */

    display->options.edges_directed_show_p = false;
    display->options.edges_arrowheads_show_p = false;

    if (display->e == NULL && ne == 1)
      setDisplayEdge (display, onlye);  /* don't rebuild the menu */
    if (display->e != NULL) {
      title = computeTitle (false, gg->current_display, gg);
      if (title) {
        gtk_window_set_title (GTK_WINDOW
                              (GGOBI_WINDOW_DISPLAY (display)->window),
                              title);
        g_free (title);
      }
    }

    display_plot (display, FULL, gg);
    break;
  case DOPT_EDGES_D:  /*-- directed: both edges and arrowheads --*/

    display->options.edges_directed_show_p = active;
    /*if (active) {
       display_edges_undirected_show (display, false);
       display_edges_arrowheads_show (display, false);
       } */
    display->options.edges_undirected_show_p = false;
    display->options.edges_arrowheads_show_p = false;


    if (display->e == NULL && ne == 1)
      setDisplayEdge (display, onlye);  /* don't rebuild the menu */
    if (display->e != NULL) {
      title = computeTitle (false, gg->current_display, gg);
      if (title) {
        gtk_window_set_title (GTK_WINDOW
                              (GGOBI_WINDOW_DISPLAY (display)->window),
                              title);
        g_free (title);
      }
    }

    display_plot (display, FULL, gg);
    break;
  case DOPT_EDGES_A:  /*-- arrowheads only --*/
    display->options.edges_arrowheads_show_p = active;
    /*if (active) {
       display_edges_directed_show (display, false);
       display_edges_undirected_show (display, false);
       } */
    display->options.edges_directed_show_p = false;
    display->options.edges_undirected_show_p = false;


    if (display->e == NULL && ne == 1)
      setDisplayEdge (display, onlye);  /* don't rebuild the menu */
    if (display->e != NULL) {
      title = computeTitle (false, gg->current_display, gg);
      if (title) {
        gtk_window_set_title (GTK_WINDOW
                              (GGOBI_WINDOW_DISPLAY (display)->window),
                              title);
        g_free (title);
      }
    }


    display_plot (display, FULL, gg);
    break;
  case DOPT_EDGES_H:/*-- arrowheads only --*/
    display->options.edges_arrowheads_show_p = false;
    display->options.edges_directed_show_p = false;
    display->options.edges_undirected_show_p = false;

    if (display->e == NULL && ne == 1)
      setDisplayEdge (display, onlye);  /* don't rebuild the menu */
    if (display->e != NULL) {
      title = computeTitle (false, gg->current_display, gg);
      if (title) {
        gtk_window_set_title (GTK_WINDOW
                              (GGOBI_WINDOW_DISPLAY (display)->window),
                              title);
        g_free (title);
      }
    }


    display_plot (display, FULL, gg);
    break;

  case DOPT_WHISKERS:
    display->options.whiskers_show_p = active;
    display_plot (display, FULL, gg);
    break;

  case DOPT_AXES:
    display->options.axes_show_p = active;

    if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
      GGobiExtendedDisplayClass *klass;
      klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
      if (klass->set_show_axes_option)
        klass->set_show_axes_option (display, active);
    }
    break;

  case DOPT_AXESLAB:
    display->options.axes_label_p = active;

    if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
      GGobiExtendedDisplayClass *klass;
      klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
      if (klass->set_show_axes_label_option)
        klass->set_show_axes_label_option (display, active);
    }
    break;

  case DOPT_AXESVALS:
    display->options.axes_values_p = active;

    if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
      GGobiExtendedDisplayClass *klass;
      klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
      if (klass->set_show_axes_values_option)
        klass->set_show_axes_values_option (display, active);
    }
    break;

/* unused
    case DOPT_AXES_C:
      display->options.axes_center_p = w->active;
    break;
    case DOPT_BUFFER:
      display->options.double_buffer_p = w->active;
    break;
    case DOPT_LINK:
      display->options.link_p = w->active;
    break;
*/
  default:
    g_printerr ("no variable is associated with %d\n", action);
  }
}


/*
 This is a start at allowing programmatic specification of
 options. Here, we set them in place (i.e. in display->options) and
 then force them to be applied.  This does not update the menus.
*/
void
set_display_options (displayd * display, GGobiSession * gg)
{
  int i;
  gboolean active = true;
  DisplayOptions *options = &display->options;
  for (i = DOPT_POINTS; i <= DOPT_WHISKERS; i++) {
    if (i == DOPT_EDGES_U || i == DOPT_EDGES_D || i == DOPT_EDGES_A ||
        i == DOPT_EDGES_H)
      if (display->edge_merge == -1)
        continue;

    switch (i) {
    case DOPT_POINTS:
      active = options->points_show_p;
      break;
    case DOPT_AXES:
      active = options->axes_show_p;
      break;
    case DOPT_AXESLAB:
      active = options->axes_label_p;
      break;
    case DOPT_AXESVALS:
      active = options->axes_values_p;
      break;
    case DOPT_EDGES_U:
      active = options->edges_undirected_show_p;
      break;
    case DOPT_EDGES_A:
      active = options->edges_arrowheads_show_p;
      break;
    case DOPT_EDGES_D:
      active = options->edges_directed_show_p;
      break;
    case DOPT_WHISKERS:
      active = options->whiskers_show_p;
      break;
    }

    set_display_option (active, i, display);
  }
}

/*-- Close a display --*/
void
display_close (displayd * display)
{
  GGobiSession *gg = GGobiFromDisplay (display);
  display_free (display, false, gg);
}


void
show_display_control_panel (displayd * display)
{
  GGobiSession *gg = GGobiFromDisplay (display);
  /* gtk_window_present(GTK_WINDOW(gg->main_window())); */
  gdk_window_raise (gg->main_window->window);
}


/*-- Called when a window is deleted from the window manager --*/
void
display_delete_cb (GtkWidget * w, GdkEvent * event, displayd * display)
{
  GGobiSession *gg = GGobiFromWidget (w, true);
  display_free (display, false, gg);
}

/*----------------------------------------------------------------------*/
/*                   End callbacks                                      */
/*----------------------------------------------------------------------*/


/*XXX consolidate this and display_alloc_init(), i.e. remove the latter! 
 This display class is really an abstract class.
*/
displayd *
ggobi_display_new (gboolean missing_p, GGobiStage * d, GGobiSession * gg)
{
  return (display_alloc_init (missing_p, d, gg));
}

void
display_set_values (displayd * display, GGobiStage * d, GGobiSession * gg)
{
  /* Should copy in the contents of DefaultOptions to create
     an indepedently modifiable configuration copied from
     the current template.
   */
  display->options = DefaultDisplayOptions;

  display->ggobi = gg;
  /* the display uses data from the end of the pipeline */
  display->d = ggobi_stage_find(d, GGOBI_MAIN_STAGE_DISPLAY);
}

displayd *
display_alloc_init (gboolean missing_p, GGobiStage * d, GGobiSession * gg)
{
  displayd *display;

  display = g_object_new (GGOBI_TYPE_WINDOW_DISPLAY, NULL);
  GGOBI_WINDOW_DISPLAY (display)->useWindow = true;
  display_set_values (display, d, gg);

  return (display);
}


gint
display_add (displayd * display, GGobiSession * gg)
{
  splotd *prev_splot = gg->current_splot;
  ProjectionMode pmode_prev = pmode_get (gg->current_display, gg);
  InteractionMode imode_prev = imode_get (gg);
  ///displayd *oldDisplay = gg->current_display;

  /* This is a safety test to avoid having a display be entered twice.
     Deactivate if we want to be slightly more efficient.             */
  if (g_list_find (gg->displays, display)) {
    g_printerr
      ("Display has already been added to the displays list of this ggobi\n");
    return (-1);
  }

  /* moved this here so that the current splot is set when configuring
     cpanels - mfl */
  if (g_list_length (display->splots)) {
    gg->current_splot = (splotd *)
      g_list_nth_data (display->splots, 0);
    display->current_splot = gg->current_splot;
    splot_set_current (gg->current_splot, on, gg);
  }

  if (GGOBI_IS_WINDOW_DISPLAY (display)
      && GGOBI_WINDOW_DISPLAY (display)->useWindow) {
    ggobi_widget_set (GGOBI_WINDOW_DISPLAY (display)->window, gg, true);
  }
  
  if (g_list_length (display->splots))
    display_set_current (display, gg);  /*-- this initializes the mode --*/
  
  gg->displays = g_list_append (gg->displays, (gpointer) display);

  /* If the tree of displays is active, add this to it. */
  display_add_tree (display);

  // setting current splot was here

  /*
   * The current display types start without signal handlers, but
   * I may need to add handlers later for some unforeseen display.
   */

  /*-- if starting from the API, or changing mode, update the mode menus --*/
  if (pmode_prev != gg->current_display->cpanel.pmode ||
      imode_prev != gg->current_display->cpanel.imode) {
    /*main_miscmenus_update (pmode_prev, imode_prev, oldDisplay, gg); */
    display_mode_menus_update (pmode_prev, imode_prev,
                               gg->current_display, gg);
  }

  /*-- Make sure the border for the previous plot is turned off --*/
  if (prev_splot != NULL) {
    prev_splot->redraw_style = QUICK;
    gtk_widget_queue_draw (prev_splot->da);
  }

  g_signal_emit (G_OBJECT (gg), GGobiSignals[DISPLAY_NEW_SIGNAL], 0, display);

  return (g_list_length (gg->displays));
}


/*
 * Remove display from the linked list of displays.  Reset
 * current_display and current_splot if necessary.
*/
void
display_free (displayd * display, gboolean force, GGobiSession * gg)
{
  splotd *sp = NULL;
  extern gint num_ggobis;
  gint count;
  displayd *dsp;

  if (force == false && sessionOptions->info->allowCloseLastDisplay)
    force = true;

  if (num_ggobis > 1 || force || g_list_length (gg->displays) > 0) {

    /* These are probably mutually exclusive and so we could use an
       else-if sequence but the results are catastrophic if we get it
       wrong so we play on the safe side of the fence.
     */

    if (display->t1d.idled) {
      tour1d_func (false, display, gg);
    }
    if (display->t1d_window)    /* XXX more to free or destroy here? */
      gtk_widget_destroy (display->t1d_window);

    if (display->t2d.idled) {
      tour2d_func (false, display, gg);
    }
    if (display->t2d_window)    /* XXX more to free or destroy here? */
      gtk_widget_destroy (display->t2d_window);

    if (display->tcorr1.idled) {
      tourcorr_func (false, display, gg);
    }
    if (display->t2d3.idled) {
      tour2d3_func (false, display, gg);
    }

/*
 * If the current splot belongs to this display, turn off its
 * event handlers before freeing all the splots belonging to this
 * display.

  This was outside and before the conditional of being able to close
  this display.
*/
    dsp = (displayd *) gg->current_splot->displayptr;
    if (dsp == display) {
      sp_event_handlers_toggle (gg->current_splot, off, display->cpanel.pmode,
                                display->cpanel.imode);
    }


    /*-- If the display tree is active, remove the corresponding entry. --*/
    tree_display_entry_remove (display, gg->display_tree.tree, gg);

    gg->displays = g_list_remove (gg->displays, display);

    /*-- if the current_display was just removed, assign a new one --*/
    /*-- list length only has to be >=0 because a display was just removed --*/
    if (display == gg->current_display) {
      if (g_list_length (gg->displays) > 0) {
        dsp = (displayd *) g_list_nth_data (gg->displays, 0);
        display_set_current (dsp, gg);

        gg->current_splot = (splotd *)
          g_list_nth_data (gg->current_display->splots, 0);
        dsp->current_splot = gg->current_splot;
        splot_set_current (gg->current_splot, on, gg);
        sp = gg->current_splot;
        if (sp != NULL) {
          sp->redraw_style = QUICK;
          gtk_widget_queue_draw (sp->da);
        }
      }
      else {
        gg->current_display = NULL;
        gg->current_splot = NULL;
      }
    }

    count = g_list_length (display->splots);

    if (GGOBI_IS_WINDOW_DISPLAY (display)) {
/*XX
      GList *l;
      for (l=display->splots; count > 0 && l; l=l->next, count--) {
        sp = (splotd *) l->data;
        splot_free (sp, display, gg);
      }
*/

      gtk_widget_destroy (GGOBI_WINDOW_DISPLAY (display)->window);
    }
  }

  /*-- If there are no longer any displays, set ggobi's mode to NULLMODE --*/
  if (g_list_length (gg->displays) == 0) {
    ggobi_full_viewmode_set (NULL_PMODE, NULL_IMODE, gg);
  }
}

void
display_free_all (GGobiSession * gg)
{
  GList *dlist;
  displayd *display;
  gint count;

  if (gg->displays == NULL)
    return;


  count = g_list_length (gg->displays);

  /* Have to count down rather than rely on dlist being non-null.
     This is because when we remove the last entry, we get garbage,
     not a null value.
   */
  for (dlist = gg->displays; count > 0 && dlist; count--) {
    gint nc;
    display = (displayd *) dlist->data;
    nc = display->d->n_cols;
    if (display == NULL)
      break;

    if (nc >= MIN_NVARS_FOR_TOUR1D && display->t1d.idled)
      g_source_remove (display->t1d.idled);
    if (nc >= MIN_NVARS_FOR_TOUR2D && display->t2d.idled)
      g_source_remove (display->t2d.idled);
    if (nc >= MIN_NVARS_FOR_COTOUR && display->tcorr1.idled)
      g_source_remove (display->tcorr1.idled);
    /* This doesn't seem to be in use.
    if (nc >= MIN_NVARS_FOR_COTOUR && display->tcorr2.idled)
      g_source_remove (display->tcorr2.idled);
    */


    /* If the second argument 'force' is true, it eliminates the
       final display.  It will work now if there is more than one
       ggobi instance running.
     */
    dlist = dlist->next;
    display_free (display, true, gg);
  }
}

void
display_set_current (displayd * new_display, GGobiSession * gg)
{
  gchar *title;

  if (new_display == NULL)
    return;

  gtk_accel_group_unlock (gg->main_accel_group);

  /* Clean up the old display first. Reset its title to show it is no
     longer active.  Clean up the control panel of the elements
     provided by this old display, in order to get ready for the
     elements provided by the new display. */

  if (gg->firsttime == false && gg->current_display &&
      GGOBI_IS_WINDOW_DISPLAY (gg->current_display)) {
    title = computeTitle (false, gg->current_display, gg);
    if (title && GGOBI_WINDOW_DISPLAY (gg->current_display)->window) {
      gtk_window_set_title (GTK_WINDOW
                            (GGOBI_WINDOW_DISPLAY (gg->current_display)->
                             window), title);
      g_free (title);
    }

    /* Now clean up the different control panel menus associated with
       this display.  Specifically, this deletes the imode and pmode
       menus.
     */
    if (GGOBI_IS_EXTENDED_DISPLAY (gg->current_display)) {
      gtk_ui_manager_remove_ui (gg->main_menu_manager, gg->mode_merge_id);
      /* Allow the extended display to override the submenu_destroy
         call.  If it doesn't provide a method, then call
         submenu_destroy. */
      void (*f) (displayd * dpy) =
        GGOBI_EXTENDED_DISPLAY_GET_CLASS (gg->current_display)->display_unset;
      if (f) {
        f (gg->current_display);
        f (gg->current_display);
      }
    }
  }

  /* Now do the setup for the new display.  */
  if (GGOBI_IS_WINDOW_DISPLAY (new_display)) {
    if (GGOBI_WINDOW_DISPLAY (new_display)->useWindow) {
      title = computeTitle (true, new_display, gg);
      if (title) {
        gtk_window_set_title (GTK_WINDOW
                              (GGOBI_WINDOW_DISPLAY (new_display)->window),
                              title);
        g_free (title);
      }
    }

    if (GGOBI_IS_EXTENDED_DISPLAY (new_display)) {
      const gchar *(*ui_get) (displayd * dpy) =
        GGOBI_EXTENDED_DISPLAY_GET_CLASS (new_display)->mode_ui_get;
      if (ui_get) {
        GError *error = NULL;
        const gchar *ui = ui_get (new_display);
        gg->mode_merge_id =
          gtk_ui_manager_add_ui_from_string (gg->main_menu_manager, ui, -1,
                                             &error);
        if (error) {
          g_message ("Could not merge main mode ui from display");
          g_error_free (error);
        }
      }
      void (*f) (displayd * dpy, GGobiSession * gg) =
        GGOBI_EXTENDED_DISPLAY_GET_CLASS (new_display)->display_set;
      if (f)
        f (new_display, gg);
    }
  }

  gg->current_display = new_display;

  g_signal_emit (G_OBJECT (gg), GGobiSignals[DISPLAY_SELECTED_SIGNAL], 0,
                 new_display);

  // replaced by listeners -- dfs
  // cpanel_set (gg->current_display, gg);
  // varpanel_show_page (gg->current_display, gg);
  // vartable_show_page (gg->current_display->d, gg);
  // varpanel_tooltips_set (gg->current_display, gg);

  gtk_accel_group_lock (gg->main_accel_group);
  gg->firsttime = false;
}

/**
   Creates a title string for a display window.

   Caller must free the return value.
 */
gchar *
computeTitle (gboolean current_p, displayd * display, GGobiSession * gg)
{
  gint n;
  gchar *title = NULL, *description;
  const char *tmp = NULL;
  GGobiStage *d = ggobi_stage_get_root(display->d);
  GGobiStage *e = display->e ? ggobi_stage_get_root(display->e) : NULL;

  if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
    tmp = ggobi_display_title_label (display);
  }

  if (d->name != NULL) {
    if (e != NULL && e->name != NULL)
      description = g_strdup_printf ("%s/%s", d->name, e->name);
    else
      description = g_strdup (d->name);
  }
  else {
    description = ggobi_input_source_get_display_name(gg->data_source);
  }

  n = strlen (tmp) + strlen (description) + 5 +
    (current_p ? strlen ("(current)") : 0);
  title = (gchar *) g_malloc0 (sizeof (gchar) * n);
  sprintf (title, "%s: %s %s", description, tmp,
           (current_p ? "(current)" : ""));
  g_free (description);

  return (title);
}


/*
 * replot all splots in all displays -- except splot, if present
*/
void
displays_plot (splotd * splot, RedrawStyle type, GGobiSession * gg)
{
  GList *dlist;
  displayd *display;

  for (dlist = gg->displays; dlist; dlist = dlist->next) {
    display = (displayd *) dlist->data;

    if (splot == NULL)
      display_plot (display, type, gg);
    else
      display_plot_allbutone (display, splot, type, gg);
  }
}

/*-- reproject and replot all splots in display --*/
void
display_tailpipe (displayd * display, RedrawStyle type, GGobiSession * gg)
{
  GList *splist = display->splots;
  splotd *sp;
  cpaneld *cpanel;

  /* this ugly block of code brought to you by the lack of plot-level stages */
  for (GList *sl = splist; sl; sl = sl->next) {
    sp = (splotd *) sl->data;
    if (sp != NULL)
      splot_points_realloc (sp);
    /*-- this is only necessary if there are variables, I think --*/
    if (GGOBI_IS_EXTENDED_SPLOT (sp)) {
      GGobiExtendedSPlotClass *klass = GGOBI_EXTENDED_SPLOT_GET_CLASS (sp);
      if (klass->alloc_whiskers)
        sp->whiskers = klass->alloc_whiskers (sp->whiskers, display->d);
      /*-- each plot type should have its own realloc routines --*/
      if (GGOBI_IS_BARCHART_SPLOT (sp)) {
        barchartSPlotd *bsp = GGOBI_BARCHART_SPLOT (sp);
        barchart_clean_init (bsp);
        barchart_recalc_counts (bsp, display->d, gg);
      }
    }
  }
  
  while (splist) {
    sp = (splotd *) splist->data;
    cpanel = &display->cpanel;
    splot_world_to_plane (cpanel, sp, gg);  /*-- includes p1d_spread_var --*/
    splot_plane_to_screen (display, cpanel, sp, gg);
    splist = splist->next;
  }

  splist = display->splots;
  while (splist) {
    sp = (splotd *) splist->data;

/*-- update transient brushing; I will also need to un-brush some points --*/
    if (display == gg->current_display &&
        sp == gg->current_splot && imode_get (gg) == BRUSH) {
      GGobiStage *d = display->d;
      if (GGOBI_IS_EXTENDED_SPLOT (sp)) {
        void (*f) (GGobiStage *, splotd *, GGobiSession *);
        GGobiExtendedSPlotClass *klass;
        klass = GGOBI_EXTENDED_SPLOT_GET_CLASS (sp);
        f = klass->splot_assign_points_to_bins;
        if (f) {
          f (d, sp, gg);        // need to exclude area plots
        }
      }
      //assign_points_to_bins (d, sp, gg); // damnit, not for area plots
    }

    if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
      void (*f) (gboolean, displayd *, splotd *, GGobiSession *);
      f = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display)->ruler_ranges_set;
      if (f) {
        f (GTK_WIDGET_VISIBLE (display->hrule) ||
           GTK_WIDGET_VISIBLE (display->vrule), display, sp, gg);
      }
    }


    splot_redraw (sp, type, gg);
    splist = splist->next;
  }
}

/*-- Reproject and plot all plots in all displays: modulo missingness --*/
void
displays_tailpipe (RedrawStyle type, GGobiSession * gg)
{
  GList *dlist;
  displayd *display;

  for (dlist = gg->displays; dlist; dlist = dlist->next) {
    display = (displayd *) dlist->data;
    display_tailpipe (display, type, gg);
  }
}

void
display_window_init (windowDisplayd * display, gint width, gint height,
                     gint bwidth, GGobiSession * gg)
{
  display->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_object_set_data (G_OBJECT (display->window),
                     "displayd", (gpointer) display);
  /* allowing shrink is considered a 'bad thing' - instead we no
     longer request a size for the drawing areas (splots), but instead
     we set a default window size and allow the splots to
     automatically fill the space - mfl */
  //gtk_window_set_policy (GTK_WINDOW (display->window), true, true, false);
  gtk_window_set_default_size (GTK_WINDOW (display->window), width, height);
  //gtk_container_set_border_width (GTK_CONTAINER (display->window), bwidth);
  g_signal_connect (G_OBJECT (display->window), "delete_event",
                    G_CALLBACK (display_delete_cb), (gpointer) display);

  ggobi_widget_set (GTK_WIDGET (display->window), gg, true);
}


gboolean
isEmbeddedDisplay (displayd * dpy)
{
  gboolean ans = false;
  ans = (GGOBI_IS_WINDOW_DISPLAY (dpy) == false
         || GGOBI_WINDOW_DISPLAY (dpy)->useWindow);

  return (ans);
}

/*
 * Since display types are unfortunately not subclasses of display,
 * we could use a method for figuring out which display types
 * support which view modes
*/
gboolean
display_type_handles_projection (displayd * display, ProjectionMode pmode)
{
  gboolean handles = false;
  ProjectionMode v = pmode;

  if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
    handles =
      GGOBI_EXTENDED_DISPLAY_GET_CLASS (display)->handles_projection (display,
                                                                      v);
  }

  return handles;
}

gboolean
display_type_handles_interaction (displayd * display, InteractionMode imode)
{
  gboolean handles = false;
  InteractionMode v = imode;

  if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
    handles =
      GGOBI_EXTENDED_DISPLAY_GET_CLASS (display)->
      handles_interaction (display, v);
  }

  return handles;
}

gboolean
display_copy_edge_options (displayd * dsp, displayd * dspnew)
{
  GtkAction *action;

  dspnew->options.edges_undirected_show_p =
    dsp->options.edges_undirected_show_p;
  action = gtk_ui_manager_get_action (dspnew->menu_manager,
                                      "/menubar/Edges/ShowUndirectedEdges");
  if (action) {
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
                                  dspnew->options.edges_undirected_show_p);
  }

  dspnew->options.edges_directed_show_p = dsp->options.edges_directed_show_p;
  action = gtk_ui_manager_get_action (dspnew->menu_manager,
                                      "/menubar/Edges/ShowDirectedEdges");
  if (action) {
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
                                  dspnew->options.edges_directed_show_p);
  }

  dspnew->options.edges_arrowheads_show_p =
    dsp->options.edges_arrowheads_show_p;
  action = gtk_ui_manager_get_action (dspnew->menu_manager,
                                      "/menubar/Edges/ShowArrowheadsOnly");
  if (action) {
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
                                  dspnew->options.edges_arrowheads_show_p);
  }

  return (dspnew->options.edges_directed_show_p ||
          dspnew->options.edges_undirected_show_p ||
          dspnew->options.edges_arrowheads_show_p);
}
