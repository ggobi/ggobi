/* display.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include "vars.h"
#include "externs.h"

#include "display_tree.h"

#include <string.h> /* for memset */

#include "print.h"

DisplayOptions DefaultDisplayOptions = {
                                         true,  /* points_show_p */
                                         true,  /* axes_show_p */
                                         false, /* axes_label_p */
                                         true, /* axes_values_p */
                                         false, /* edges_undirected_show_p */
                                         false, /* edges_arrowheads_show_p */
                                         false, /* edges_directed_show_p */
                                         true,  /* whiskers_show_p*/
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
displays_print (ggobid *gg) {
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
display_plot (displayd *display, RedrawStyle type, ggobid *gg) 
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
display_plot_allbutone (displayd *display, splotd *splot,
  RedrawStyle type, ggobid *gg)
{
  GList *slist;
  splotd *sp;

  for (slist = display->splots; slist; slist = slist->next) {
    sp = (splotd *) slist->data;
    if (sp == NULL)
      ;
    else if (splot == NULL || sp != splot)
      splot_redraw (sp, type, gg); 
  }
}

/*----------------------------------------------------------------------*/
/*           Callbacks common to multiple display types                 */
/*----------------------------------------------------------------------*/

void
display_options_cb (GtkCheckMenuItem *w, guint action) 
{
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(w), true);
  displayd *display = (displayd *)
    gtk_object_get_data (GTK_OBJECT (w), "display");
  GtkWidget *ww;

  switch (action) {
    case DOPT_POINTS:
      display->options.points_show_p = w->active;
      display_plot (display, FULL, gg);
    break;
    case DOPT_EDGES_U:  /*-- undirected: edges only --*/
      if (display->e == NULL) edgeset_add (display);
      if (display->e != NULL) {
        display->options.edges_undirected_show_p = w->active;

        if (!w->active && display->options.edges_directed_show_p) { 
          ww = widget_find_by_name (display->edge_menu,
            "DISPLAY MENU: show directed edges");
          if (ww) {
            gtk_check_menu_item_set_active ((GtkCheckMenuItem *) ww, false);
          }
        }
      }

      display_plot (display, FULL, gg);
    break;
    case DOPT_EDGES_A:  /*-- arrowheads only --*/
      if (display->e == NULL) edgeset_add (display);
      if (display->e != NULL) {
        display->options.edges_arrowheads_show_p = w->active;
        gtk_object_set_data (GTK_OBJECT (w), "propagate",
          GINT_TO_POINTER(false));
        if (!w->active && display->options.edges_directed_show_p) { 
          ww = widget_find_by_name (display->edge_menu,
            "DISPLAY MENU: show directed edges");
          if (ww) {
            gtk_check_menu_item_set_active ((GtkCheckMenuItem *) ww, false);
          }
        }
      }

      display_plot (display, FULL, gg);
    break;
    case DOPT_EDGES_D:  /*-- directed: both edges and arrowheads --*/
      if (display->e == NULL) edgeset_add (display);
      if (display->e != NULL) {
        display->options.edges_directed_show_p = w->active;

        if (!w->active && display->options.edges_undirected_show_p) { 
          ww = widget_find_by_name (display->edge_menu,
            "DISPLAY MENU: show undirected edges");
          if (ww) {
            gtk_check_menu_item_set_active ((GtkCheckMenuItem *) ww, false);
          }
        }
        if (!w->active && display->options.edges_arrowheads_show_p) { 
          ww = widget_find_by_name (display->edge_menu,
            "DISPLAY MENU: show arrowheads");
          if (ww) {
            gtk_check_menu_item_set_active ((GtkCheckMenuItem *) ww, false);
          }
        }
      }

      display_plot (display, FULL, gg);
    break;
    case DOPT_WHISKERS:
      display->options.whiskers_show_p = w->active;
      display_plot (display, FULL, gg);
    break;
/*
    case DOPT_MISSINGS:
      if (!display->missing_p && d->nmissing > 0) {
        display->options.missings_show_p = w->active;

        if (display->displaytype == parcoords) {
          GList *splist;
          splotd *sp;
          for (splist = display->splots; splist; splist = splist->next) {
            sp = (splotd *) splist->data;
            sp_whiskers_make (sp, display, gg);
          }
        } else if (display->displaytype == tsplot) {
          GList *splist;
          splotd *sp;
          for (splist = display->splots; splist; splist = splist->next) {
            sp = (splotd *) splist->data;
            tsplot_whiskers_make (sp, display, gg);
          }
        }

        display_plot (display, FULL, gg);
      }
    break;
*/

    case DOPT_AXES:
      display->options.axes_show_p = w->active;

      if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
        GtkGGobiExtendedDisplayClass *klass;
        klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
        if(klass->set_show_axes_option)
          klass->set_show_axes_option(display, w->active);
      }
    break;

    case DOPT_AXESLAB:
      display->options.axes_label_p = w->active;

      if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
        GtkGGobiExtendedDisplayClass *klass;
        klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
        if(klass->set_show_axes_label_option)
          klass->set_show_axes_label_option(display, w->active);
      }      
    break;

    case DOPT_AXESVALS:
      display->options.axes_values_p = w->active;

      if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
        GtkGGobiExtendedDisplayClass *klass;
        klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
        if(klass->set_show_axes_values_option)
          klass->set_show_axes_values_option(display, w->active);
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

void
display_print_cb (displayd *display, guint action, GtkWidget *w) 
{
  ggobid *gg;
  gg = display->ggobi;

  if(gg->printOptions == NULL) {
    gg->printOptions = getDefaultPrintOptions(NULL);
  }

  if(DefaultPrintHandler.callback)
    (*DefaultPrintHandler.callback)(gg->printOptions, display,
      display->ggobi, &DefaultPrintHandler);
}

/*-- Called when a plot window is closed from the menu --*/
void
display_close_cb (displayd *display, guint action, GtkWidget *w) 
{
  ggobid *gg = GGobiFromDisplay (display);
  display_free (display, false, gg);
}

/*-- Called when a window is deleted from the window manager --*/
void
display_delete_cb (GtkWidget *w, GdkEvent *event, displayd *display) 
{
  ggobid *gg = GGobiFromWidget (w, true);
  display_free (display, false, gg);
}

/*----------------------------------------------------------------------*/
/*                   End callbacks                                      */
/*----------------------------------------------------------------------*/


/*XXX consolidate this and display_alloc_init(), i.e. remove the latter! 
 This display class is really a virtual class.
*/
displayd *
gtk_ggobi_display_new(gboolean missing_p, datad *d, ggobid *gg)
{
  return(display_alloc_init(missing_p, d, gg));
}

void
display_set_values(displayd *display, datad *d, ggobid *gg)
{
  /* Should copy in the contents of DefaultOptions to create
     an indepedently modifiable configuration copied from
     the current template.
   */
  display->options = DefaultDisplayOptions;

  display->ggobi = gg;
  display->d = d;
}

displayd *
display_alloc_init (gboolean missing_p, datad *d, ggobid *gg)
{
  displayd *display;

  display = gtk_type_new(GTK_TYPE_GGOBI_WINDOW_DISPLAY);
  display_set_values(display, d, gg);

  return (display);
}


gint
display_add (displayd *display, ggobid *gg)
{
  splotd *prev_splot = gg->current_splot;
  PipelineMode prev_viewmode = viewmode_get (gg);
  displayd *oldDisplay = gg->current_display;

   /* This is a safety test to avoid having a display be entered twice.
      Deactivate if we want to be slightly more efficient.             */  
  if(g_list_find(gg->displays, display)) {
    g_printerr("Display has already been added to the displays list of this ggobi\n");
    return(-1);
  }

  if (GTK_IS_GGOBI_WINDOW_DISPLAY(display)) {
    GGobi_widget_set(GTK_GGOBI_WINDOW_DISPLAY(display)->window, gg, true);
    if(g_list_length(display->splots))
          display_set_current (display, gg);  /*-- this may initialize the mode --*/
  }
  gg->displays = g_list_append (gg->displays, (gpointer) display);

    /* If the tree of displays is active, add this to it. */
  display_add_tree(display, -1, gg->display_tree.tree, gg);

  if(g_list_length(display->splots)) {
     gg->current_splot = (splotd *)
       g_list_nth_data (gg->current_display->splots, 0);
     display->current_splot = gg->current_splot;
     splot_set_current (gg->current_splot, on, gg);
  }


  /*
   * The current display types start without signal handlers, but
   * I may need to add handlers later for some unforeseen display.
  */
  /* don't activate */
  viewmode_set (gg->current_display->cpanel.viewmode, gg); 

  /*-- if starting from the API, or changing mode, update the mode menus --*/
  if (prev_viewmode != gg->current_display->cpanel.viewmode) {
    viewmode_submenus_update (prev_viewmode, oldDisplay, gg);
  }

  /*-- Make sure the border for the previous plot is turned off --*/
  if (prev_splot != NULL) {
    prev_splot->redraw_style = QUICK;
    gtk_widget_queue_draw (prev_splot->da);
  }

  return (g_list_length (gg->displays));
}


/*
 * Remove display from the linked list of displays.  Reset
 * current_display and current_splot if necessary.
*/ 
void
display_free (displayd* display, gboolean force, ggobid *gg) 
{
  splotd *sp = NULL;
  extern gint num_ggobis;
  gint count;
  displayd *dsp;

  if(force == false && sessionOptions->info->allowCloseLastDisplay)
      force = true;

  if (num_ggobis > 1 || force || g_list_length (gg->displays) > 0) {

    /* These are probably mutually exclusive 
       and so we could use an else-if sequence
       but the results are catastrophic if we get it
       wrong so we play on the safe side of the fence.
     */
    if(display->t2d.idled) {
      tour2d_func(false, display, gg);
    }
    if(display->t1d.idled) {
      tour1d_func(false, display, gg);
    }
    if(display->tcorr1.idled) {
      tourcorr_func(false, display, gg);
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
      sp_event_handlers_toggle (gg->current_splot, off);
    }


    /*-- If the display tree is active, remove the corresponding entry. --*/
    tree_display_entry_remove (display, gg->display_tree.tree, gg); 

    gg->displays = g_list_remove (gg->displays, display);

    /*-- if the current_display was just removed, assign a new one --*/
    /*-- list length only has to be >=0 because a display was just removed --*/
    if (display == gg->current_display) {
     if(g_list_length (gg->displays) > 0) {
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
     } else {
       gg->current_display = NULL;
       gg->current_splot = NULL;
     }
    }

    count = g_list_length (display->splots);

    if (GTK_IS_GGOBI_WINDOW_DISPLAY(display)) {
/*XX
      GList *l;
      for (l=display->splots; count > 0 && l; l=l->next, count--) {
        sp = (splotd *) l->data;
        splot_free (sp, display, gg);
      }
*/

     gtk_widget_destroy (GTK_GGOBI_WINDOW_DISPLAY(display)->window);
    }
#if 0 /*XX */
    gtk_object_destroy(GTK_OBJECT(display));
#endif

  } else
    quick_message ("Sorry, you can't delete the only display\n", false);
}

void
display_free_all (ggobid *gg) {
  GList *dlist;
  displayd *display;
  gint count;

  if(gg->displays == NULL)
      return;


  count = g_list_length (gg->displays);

  /* Have to count down rather than rely on dlist being non-null.
     This is because when we remove the last entry, we get garbage,
     not a null value.
   */
  for (dlist = gg->displays; count > 0 && dlist; count--)
  {
    gint nc;
    display = (displayd *) dlist->data;
    nc = display->d->ncols;
    if (display == NULL)
      break;

    if(nc >= MIN_NVARS_FOR_TOUR1D && display->t1d.idled)
      gtk_idle_remove(display->t1d.idled);
    if(nc >= MIN_NVARS_FOR_TOUR2D && display->t2d.idled)
      gtk_idle_remove(display->t2d.idled);
    if(nc >= MIN_NVARS_FOR_COTOUR && display->tcorr1.idled)
      gtk_idle_remove(display->tcorr1.idled);
    if(nc >= MIN_NVARS_FOR_COTOUR && display->tcorr2.idled)
      gtk_idle_remove(display->tcorr2.idled);


     /* If the second argument 'force' is true, it eliminates the
        final display.
        It will work now if there is more than one ggobi instance
        running.
      */
    dlist = dlist->next;
    display_free (display, true, gg); 
  }
}

void
display_set_current (displayd *new_display, ggobid *gg) 
{
  gchar *title;

  if (new_display == NULL)
    return;

  gtk_accel_group_unlock (gg->main_accel_group);

    /* Cleanup the old display first. Reset its title to show it is no longer active.
       Clean up the control panel of the elements provided by this old display,
       in order to get ready for the elements provided by the new display. */

  if (gg->firsttime == false && gg->current_display &&
      GTK_IS_GGOBI_WINDOW_DISPLAY(gg->current_display))
  {
    title = computeTitle (false, gg->current_display, gg);
    if (title) {
      gtk_window_set_title (GTK_WINDOW (GTK_GGOBI_WINDOW_DISPLAY(gg->current_display)->window), title);
      g_free (title); 
    }

       /* Now cleanup the different control panel menus associated with this display.
          Specifically, this gets rid of the ViewMode menu.
        */
    if(GTK_IS_GGOBI_EXTENDED_DISPLAY(gg->current_display)) {
       /* Allow the extended display to override the submenu_destroy call.
          If it doesn't provide a method, then call submenu_destroy. */
      void (*f)(displayd *dpy, GtkWidget *) =
        GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(gg->current_display)->klass)->display_unset;
      if(f)
        f(gg->current_display, gg->viewmode_item);
      else
        submenu_destroy (gg->viewmode_item); /* default if no method provided. */
    }
  }


  /* Now do the setup for the new display.  */
  if (GTK_IS_GGOBI_WINDOW_DISPLAY(new_display)) {
    title = computeTitle (true, new_display, gg);
    if (title) {
      gtk_window_set_title (GTK_WINDOW (GTK_GGOBI_WINDOW_DISPLAY(new_display)->window), title);   
      g_free (title); 
    }

    if(GTK_IS_GGOBI_EXTENDED_DISPLAY(new_display)) {
       /* Allow the extended display to override the submenu_destroy call.
          If it doesn't provide a method, then call submenu_destroy. */
      void (*f)(displayd *dpy, ggobid *gg) =
        GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(new_display)->klass)->display_set;
      if(f)
        f(new_display, gg);
    }
  }

  gg->current_display = new_display;
  cpanel_set (gg->current_display, gg);

  /*
   * if the datad for the new current display doesn't match that
   * of the previous current display, move the variable selection
   * panel notebook to the appropriate tab.  Do the same thing for
   * the variable manipulation table.
  */
  varpanel_show_page (gg->current_display, gg);
  vartable_show_page (gg->current_display, gg);

  varpanel_tooltips_set (gg->current_display, gg);

  gtk_accel_group_lock (gg->main_accel_group);
  gg->firsttime = false;
}

/**
   Creates a title string for a display window.

   Caller must free the return value.
 */
gchar *
computeTitle (gboolean current_p, displayd *display, ggobid *gg)
{
  gint n;
  gchar *title = NULL, *description;
  const char *tmp = NULL, *stars;

  if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
    tmp = gtk_display_title_label(display);
  }

  if (display->d->name != NULL) {
    description = g_strdup(display->d->name);
  } else {
    description = GGOBI (getDescription)(gg);
  }

  stars = current_p ? "***" : "";
  n = strlen (tmp) + strlen (description) + 5 + (current_p ? strlen(stars)*2 : 0);
  title = (gchar *) g_malloc(sizeof(gchar) * n);
  memset (title, '\0', n);
  sprintf (title, "%s: %s %s %s", description, stars, tmp, stars);
  g_free (description);

  return (title);
}


/*
 * replot all splots in display -- except splot, if present
*/
void
displays_plot (splotd *splot, RedrawStyle type, ggobid *gg) {
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
display_tailpipe (displayd *display, RedrawStyle type, ggobid *gg) 
{
  GList *splist = display->splots;
  splotd *sp;
  cpaneld *cpanel;

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
        sp == gg->current_splot &&
        viewmode_get (gg) == BRUSH)
    {
      datad *d = display->d;
      assign_points_to_bins (d, gg);
    }

    if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
      void (*f)(gboolean, displayd *, splotd *, ggobid *);
      f = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass)->ruler_ranges_set;
      if(f)
        f(true, display, sp, gg);
    }


    splot_redraw (sp, type, gg);
    splist = splist->next;
  }
}

/*-- Reproject and plot all plots in all displays: modulo missingness --*/
void
displays_tailpipe (RedrawStyle type, ggobid *gg) {
  GList *dlist;
  displayd *display;

  for (dlist = gg->displays; dlist; dlist = dlist->next) {
    display = (displayd *) dlist->data;
    display_tailpipe (display, type, gg);
  }
}

void
display_window_init (windowDisplayd *display, gint width, ggobid *gg)
{
  display->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (display->window),
                       "displayd",
                       (gpointer) display);
  gtk_window_set_policy (GTK_WINDOW (display->window), true, true, false);
  gtk_container_set_border_width (GTK_CONTAINER (display->window), width);
  gtk_signal_connect (GTK_OBJECT (display->window), "delete_event",
                      GTK_SIGNAL_FUNC (display_delete_cb), (gpointer) display);

  GGobi_widget_set (GTK_WIDGET (display->window), gg, true);
}


gboolean
isEmbeddedDisplay (displayd *dpy)
{
  gboolean ans = false;
  ans = (GTK_IS_GGOBI_WINDOW_DISPLAY(dpy) == false);

  return (ans);
}

/*
 * Since display types are unfortunately not subclasses of display,
 * we could use a method for figuring out which display types
 * support which view modes
*/
gboolean
display_type_handles_action (displayd *display, PipelineMode viewmode) 
{
  gboolean handles = false;
  PipelineMode v = viewmode;

  if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
    handles = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass)->handles_action(display, v);
  } 

  return handles;
}

gboolean
display_copy_edge_options (displayd *dsp, displayd *dspnew)
{
  GtkWidget *item;

  dspnew->options.edges_undirected_show_p =
    dsp->options.edges_undirected_show_p;
  item = widget_find_by_name (dspnew->edge_menu,
            "DISPLAY MENU: show undirected edges");
  if (item) {
    gtk_check_menu_item_set_active ((GtkCheckMenuItem *) item,
      dspnew->options.edges_undirected_show_p);
  }

  dspnew->options.edges_directed_show_p =
    dsp->options.edges_directed_show_p;
  item = widget_find_by_name (dspnew->edge_menu,
            "DISPLAY MENU: show directed edges");
  if (item)
    gtk_check_menu_item_set_active ((GtkCheckMenuItem *) item,
      dspnew->options.edges_directed_show_p);

  dspnew->options.edges_arrowheads_show_p =
    dsp->options.edges_arrowheads_show_p;
  item = widget_find_by_name (dspnew->edge_menu,
            "DISPLAY MENU: show arrowheads");
  if (item)
    gtk_check_menu_item_set_active ((GtkCheckMenuItem *) item,
      dspnew->options.edges_arrowheads_show_p);

  return (dspnew->options.edges_directed_show_p || 
          dspnew->options.edges_undirected_show_p ||
          dspnew->options.edges_arrowheads_show_p);
}
