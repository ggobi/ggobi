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
                                         false, /* edges_undirected_show_p */
                                         false, /* edges_arrowheads_show_p */
                                         false, /* edges_directed_show_p */
                                         true,  /* whiskers_show_p*/
                                         true,  /* missings_show_p  */
                                         true,  /* axes_show_p */
                                         true,  /* axes_center_p */
                                         true,  /* double_buffer_p */
                                         true   /* link_p */
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
display_plot (displayd *display, RedrawStyle type, ggobid *gg) {
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
  datad *d = display->d;
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
    case DOPT_MISSINGS:  /*-- in scatterplot, scatmat, parcoords, tsplot --*/
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
          extern void tsplot_whiskers_make (splotd *, displayd *, ggobid *);
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

    case DOPT_AXES:
      display->options.axes_show_p = w->active;

      if (display->displaytype == scatterplot) {
        switch (display->cpanel.projection) {
          case XYPLOT:
            if (display->hrule != NULL) {
              scatterplot_show_vrule (display, w->active);
              scatterplot_show_hrule (display, w->active);
            }
          break;
          case P1PLOT:
            if (display->hrule != NULL) {
              if (display->p1d_orientation == VERTICAL)
                scatterplot_show_vrule (display, w->active);
              else
                scatterplot_show_hrule (display, w->active);
            }
          case TOUR1D:
          case TOUR2D:
          case COTOUR:
            display_plot (display, QUICK, gg);
          break;
          default:
          break;
        }
      }
      break;

    case DOPT_AXES_C:
      display->options.axes_center_p = w->active;
      break;
    case DOPT_BUFFER:
      display->options.double_buffer_p = w->active;
      break;
    case DOPT_LINK:
      display->options.link_p = w->active;
      break;
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

displayd *
display_alloc_init (enum displaytyped type, gboolean missing_p,
  datad *d, ggobid *gg)
{
  displayd *display = (displayd *) g_malloc (sizeof (displayd));
  display->displaytype = type; 
  display->missing_p = missing_p;

  display->p1d_orientation = VERTICAL;

  /* Copy in the contents of DefaultOptions to create
     an indepedently modifiable configuration copied from
     the current template.
   */
  display->options = DefaultDisplayOptions;

  display->ggobi = gg;
  display->d = d;
  display->e = NULL;
  display->embeddedIn = NULL;

  display->t1d_manip_var = -1;
  display->t2d_manip_var = -1;
  display->tc1_manip_var = -1;
  display->tc2_manip_var = -1;

  display->t1d_pp_pixmap = NULL;
  display->t2d_pp_pixmap = NULL;

  return (display);
}

displayd *
display_create (gint displaytype, gboolean missing_p, datad *d, ggobid *gg)
{
  displayd *display;
  gint *selected_vars, nselected_vars = 0;

  if (d == NULL || d->nrows == 0)  /*-- if used before we have data --*/
    return (NULL);

  /*-- if trying to make a missing values plot without missing data --*/
  if (missing_p && d->nmissing == 0) 
    return (NULL);

  /*-- find out what variable are selected in the var statistics panel --*/
  selected_vars = (gint *) g_malloc (d->ncols * sizeof (gint));
  nselected_vars = selected_cols_get (selected_vars, d, gg);

  /*
   * Turn off event handlers, remove submenus, and redraw the
   * previous plot without a border.
  */
  splot_set_current (gg->current_splot, off, gg);

  switch (displaytype) {

    case 0:
      display = scatterplot_new (missing_p, NULL, d, gg);
      break;

    case 1:
      display = scatmat_new (missing_p,
        nselected_vars, selected_vars, nselected_vars, selected_vars,
        d, gg);
      break;

    case 2:
      /*
       * testing a method to allow variables to be specified
       * before plotting; this is redundant with the variable
       * statistics panel, though, and also the ability to
       * reorder variables would be awfully useful.
      */
      /* vardialog_open (gg, "Select variables for plotting"); */

      display = parcoords_new (false, nselected_vars, selected_vars, d, gg);
      break;

    case 3:
      display = tsplot_new (false, nselected_vars, selected_vars, d, gg);
      break;

    default:
      break;
  }

  display_add (display, gg);

  varpanel_refresh (gg);

  g_free (selected_vars);

  return (display);
}

gint
display_add (displayd *display, ggobid *gg)
{
  splotd *prev_splot = gg->current_splot;
  PipelineMode prev_viewmode = viewmode_get (gg);

  if (isEmbeddedDisplay(display) == false) {
    GGobi_widget_set(display->window, gg, true);
    display_set_current (display, gg);  /*-- this may initialize the mode --*/
  }
  gg->displays = g_list_append (gg->displays, (gpointer) display);

    /* If the tree of displays is active, add this to it. */
  display_add_tree(display, -1, gg->display_tree.tree, gg);

  gg->current_splot = (splotd *)
    g_list_nth_data (gg->current_display->splots, 0);
  display->current_splot = gg->current_splot;
  splot_set_current (gg->current_splot, on, gg);


  /*
   * The current display types start without signal handlers, but
   * I may need to add handlers later for some unforeseen display.
  */
  /* don't activate */
  viewmode_set (gg->current_display->cpanel.viewmode, gg); 


  /*-- if starting from the API, the first mode menu needs to be shown --*/
  if (prev_viewmode == NULLMODE)
    viewmode_submenus_update (prev_viewmode, gg);

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
display_free (displayd* display, gboolean force, ggobid *gg) {
  GList *l;
  splotd *sp = NULL;
  extern gint num_ggobis;
  gint count;
  displayd *dsp;
/*
 * If the current splot belongs to this display, turn off its
 * event handlers before freeing all the splots belonging to this
 * display.
*/
  dsp = (displayd *) gg->current_splot->displayptr;
  if (dsp == display) {
     sp_event_handlers_toggle (gg->current_splot, off);
  }

  if (num_ggobis > 1 || force || g_list_length (gg->displays) > 1) {

    /*-- If the display tree is active, remove the corresponding entry. --*/
    tree_display_entry_remove (display, gg->display_tree.tree, gg); 

    gg->displays = g_list_remove (gg->displays, display);

    /*-- if the current_display was just removed, assign a new one --*/
    /*-- list length only has to be >=0 because a display was just removed --*/
    if (display == gg->current_display && (g_list_length (gg->displays) > 0)) {
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

    count = g_list_length (display->splots);
    if (isEmbeddedDisplay (display) == false) {
      for (l=display->splots; count > 0 && l; l=l->next, count--) {
        sp = (splotd *) l->data;
        splot_free (sp, display, gg);
      }

     gtk_widget_destroy (display->window);
    }
    g_free (display);
  } else
    quick_message ("Sorry, you can't delete the only display\n", false);
}

void
display_free_all (ggobid *gg) {
  GList *dlist;
  displayd *display;
  gint count = g_list_length (gg->displays);

  /* Have to count down rather than rely on dlist being non-null.
     This is because when we remove the last entry, we get garbage,
     not a null value.
   */
  for (dlist = gg->displays; count > 0 && dlist; dlist = dlist->next, count--)
  {
    /*    display = (displayd *) dlist->data; */
    display = (displayd*) g_list_nth_data (gg->displays,count-1);

    if(display->t1d.idled)
      gtk_idle_remove(display->t1d.idled);
    if(display->t2d.idled)
      gtk_idle_remove(display->t2d.idled);
    if(display->tcorr1.idled)
      gtk_idle_remove(display->tcorr1.idled);
    if(display->tcorr2.idled)
      gtk_idle_remove(display->tcorr2.idled);
    if(display->tour_idled)
      gtk_idle_remove(display->tour_idled);


     /* If the second argument 'force' is true, it eliminates the
        final display.
        It will work now if there is more than one ggobi instance
        running.
      */
    display_free (display, true, gg); 
  }
}

void
display_set_current (displayd *new_display, ggobid *gg) 
{
  gchar *title;
  extern void varpanel_show_page (displayd*, ggobid*);

  if (new_display == NULL)
    return;

  gtk_accel_group_unlock (gg->main_accel_group);

  if (gg->firsttime == false &&
      isEmbeddedDisplay (gg->current_display) == false)
  {
    title = computeTitle (false, gg->current_display, gg);
    if (title) {
      gtk_window_set_title (GTK_WINDOW (gg->current_display->window), title);
      g_free (title); 
    }

    switch (gg->current_display->displaytype) {
      case scatterplot:
        submenu_destroy (gg->viewmode_item);
      break;
      case scatmat:
        submenu_destroy (gg->viewmode_item);
      break;
      case parcoords:
        submenu_destroy (gg->viewmode_item);
      break;
      case tsplot:
        submenu_destroy (gg->viewmode_item);
      break;
      case unknown_display_type:
/**/    return;
      break;
    }
  }

  if (isEmbeddedDisplay (new_display) == false) {
    title = computeTitle (true, new_display, gg);
    if (title) {
      gtk_window_set_title (GTK_WINDOW (new_display->window), title);   
      g_free (title); 
    }

    switch (new_display->displaytype) {
      case scatterplot:
        scatterplot_mode_menu_make (gg->main_accel_group,
          (GtkSignalFunc) viewmode_set_cb, gg, true);
        gg->viewmode_item = submenu_make ("_ViewMode", 'V',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->viewmode_item),
                                   gg->app.scatterplot_mode_menu); 
        submenu_insert (gg->viewmode_item, gg->main_menubar, 2);
      break;

      case scatmat:
        scatmat_mode_menu_make (gg->main_accel_group,
          (GtkSignalFunc) viewmode_set_cb, gg, true);
        gg->viewmode_item = submenu_make ("_ViewMode", 'V',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->viewmode_item),
                                   gg->app.scatmat_mode_menu); 
        submenu_insert (gg->viewmode_item, gg->main_menubar, 2);
      break;

      case parcoords:
        parcoords_mode_menu_make (gg->main_accel_group,
          (GtkSignalFunc) viewmode_set_cb, gg, true);
        gg->viewmode_item = submenu_make ("_ViewMode", 'V',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->viewmode_item),
                                   gg->parcoords.mode_menu); 
        submenu_insert (gg->viewmode_item, gg->main_menubar, 2);
      break;

      case tsplot:
        tsplot_mode_menu_make (gg->main_accel_group,
          (GtkSignalFunc) viewmode_set_cb, gg, true);
        gg->viewmode_item = submenu_make ("_ViewMode", 'V',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->viewmode_item),
                                   gg->tsplot.mode_menu); 
        submenu_insert (gg->viewmode_item, gg->main_menubar, 2);
      break;
      default:
      break;
    }
  }

  gg->current_display = new_display;
  cpanel_set (gg->current_display, gg);

  /*
   * if the datad for the new current display doesn't match that
   * of the previous current display, move the variable selection
   * panel notebook to the appropriate tab.
  */
  varpanel_show_page (gg->current_display, gg);

  varpanel_tooltips_set (gg);

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
  const char *tmp = NULL;

  switch (display->displaytype) {
    case scatterplot:
      if (current_p)
        tmp = display->missing_p ?   "*** scatterplot display (missings) *** " 
                                  :   "*** scatterplot display ***";
      else
        tmp = display->missing_p ?   "scatterplot display (missings) " 
                                  :   "scatterplot display ";
    break;
    case scatmat:
      if (current_p)
        tmp = display->missing_p ?   "*** scatterplot matrix (missings) *** " 
                                 :   "*** scatterplot matrix ***";
      else
        tmp = display->missing_p ?   "scatterplot matrix (missings) " 
                                 :   "scatterplot matrix ";
    break;
    case parcoords:
      if (current_p)
        tmp = "*** parallel coordinates display *** " ;
      else 
        tmp = "parallel coordinates display " ;
    break;
    case tsplot:
      if (current_p)
        tmp = "*** time series display *** " ;
      else 
        tmp = "time series display display " ;
    break;

    default:
    break;
  }

  if (display->d->name != NULL) {
    description = g_strdup(display->d->name);
  } else {
    description = GGOBI (getDescription)(gg);
  }

  n = strlen (tmp) + strlen (description) + 4;
  title = (gchar *) g_malloc(sizeof(gchar) * n);
  memset (title, '\0', n);
  sprintf (title, "%s: %s", description, tmp);
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

    if (splot == NULL) {
      display_plot (display, type, gg);
    }
    else
      display_plot_allbutone (display, splot, type, gg);
  }
}

/*-- reproject and replot all splots in display --*/
void
display_tailpipe (displayd *display, RedrawStyle type, ggobid *gg) {
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

    if (display->displaytype == scatterplot)
      ruler_ranges_set (false, display, sp, gg);

    splot_redraw (sp, type, gg);
    splist = splist->next;
  }
}

/*-- Reproject and plot all plots in all displays: modulo missingness --*/
void
displays_tailpipe (gint which, RedrawStyle type, ggobid *gg) {
  GList *dlist;
  displayd *display;
  gboolean redisplay = true;

  for (dlist = gg->displays; dlist; dlist = dlist->next) {
    display = (displayd *) dlist->data;

    if (which != REDISPLAY_ALL) {
      redisplay = ((which == REDISPLAY_MISSING && display->missing_p) ||
                   (which == REDISPLAY_PRESENT && !display->missing_p));
    }

    if (redisplay)
      display_tailpipe (display, type, gg);
  }
}

void
display_window_init (displayd *display, gint width, ggobid *gg)
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
  if (dpy->embeddedIn != NULL)
    ans = true;

  return (ans);
}

/*
 * Since display types are unfortunately not subclasses of display,
 * we could use a method for figuring out which display types
 * support which view modes
*/
gboolean
display_type_handles_action (displayd *display, PipelineMode viewmode) {
  gint dtype = display->displaytype;
  gboolean handles = false;
  PipelineMode v = viewmode;

  if (dtype == scatterplot) { 
    /*-- handles all view modes, but watch out for display types --*/
    if (v != SCATMAT && v != PCPLOT && v != TSPLOT)
      handles = true;
  } else if (dtype == scatmat) {
    if (v == SCALE || v == BRUSH || v == IDENT || v == MOVEPTS || v == SCATMAT)
      handles = true;
  } else if (dtype == parcoords) {
    if (v == BRUSH || v == IDENT || v == PCPLOT)
      handles = true;
  } else if (dtype == tsplot) {
    if (v == BRUSH || v == IDENT || v == TSPLOT)
      handles = true;
  }

  return handles;
}
