/* display.c */

#include <gtk/gtk.h>
#include <strings.h>

#include "vars.h"
#include "externs.h"

#include "display_tree.h"


DisplayOptions DefaultDisplayOptions = {
                                         true,  /* points_show_p */
                                         false, /* segments_directed_show_p */
                                         false, /* segments_undirected_show_p */
                                         true,  /* segments_show_p*/
                                         true,  /* missings_show_p  */
                                         false, /* gridlines_show_p */
                                         true,  /* axes_show_p */
                                         true,  /* axes_center_p */
                                         true,  /* double_buffer_p */
                                         true   /* link_p */
                                       };


/*----------------------------------------------------------------------*/
/*               Drawing routines                                       */
/*----------------------------------------------------------------------*/

/*-- replot all splots in display --*/
void
display_plot (displayd *display, guint type, ggobid *gg) {
  GList *slist;
  splotd *sp;

  for (slist = display->splots; slist; slist = slist->next) {
    sp = (splotd *) slist->data;
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

  switch (action) {
    case DOPT_POINTS:
      display->options.points_show_p = w->active;
      display_plot (display, FULL, gg);
      break;
    case DOPT_SEGS_D:
      display->options.segments_directed_show_p = w->active;
      display_plot (display, QUICK, gg);
      break;
    case DOPT_SEGS_U:
      display->options.segments_undirected_show_p = w->active;
      display_plot (display, QUICK, gg);
      break;
    case DOPT_SEGS:
      display->options.segments_show_p = w->active;
      display_plot (display, FULL, gg);
      break;
    case DOPT_MISSINGS:  /*-- only in scatmat and parcoords --*/
      if (!display->missing_p && gg->nmissing > 0) {
        display->options.missings_show_p = w->active;

        if (display->displaytype == parcoords) {
          GList *splist;
          splotd *sp;
          for (splist = display->splots; splist; splist = splist->next) {
            sp = (splotd *) splist->data;
            sp_whiskers_make (sp, display, gg);
          }
        }

        display_plot (display, FULL, gg);
      }
      break;
    case DOPT_GRIDLINES:
      display->options.gridlines_show_p = w->active;
      break;
    case DOPT_AXES:
      display->options.axes_show_p = w->active;
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
display_print_cb (displayd *display, guint action, GtkWidget *w) {
}

/*
  Called when a plot window is closed via the "Close"
  menu item.
 */
void
display_close_cb (displayd *display, guint action, GtkWidget *w) 
{
  ggobid *gg = GGobiFromDisplay(display);

  display_free (display, false, gg);
}

void
display_delete_cb (GtkWidget *w, GdkEvent *event, displayd *display) 
{
  ggobid *gg = GGobiFromWidget(w, true);
  display_free (display, false, gg);
}

/*----------------------------------------------------------------------*/
/*                   End callbacks                                      */
/*----------------------------------------------------------------------*/

void
display_new (ggobid *gg, guint action, GtkWidget *widget)
{
 display_create (action, gg);
}

displayd *
display_create (guint action, ggobid *gg)
{
  displayd *display;

  if (gg->nrows == 0)  /*-- if used before we have data --*/
    return (NULL);

  /*
   * Turn off event handlers, remove submenus, and redraw the
   * previous plot without a border.
  */
  splot_set_current (gg->current_splot, off, gg);

  switch (action) {

    case 0:
      display = scatterplot_new (false, NULL, gg);
      break;

    case 1:
      display = scatmat_new (false, NULL, 0, 0, gg);
      break;

    case 2:
      display = parcoords_new (false, NULL, 0, gg);
      break;

    case 3:  /*-- scatterplot of missing values --*/
      if (gg->nmissing)
        display = scatterplot_new (true, NULL, gg);

      break;

    case 4:  /*-- scatterplot matrix of missing values --*/
      if (gg->nmissing)
        display = scatmat_new (true, NULL, 0, 0, gg);
      break;

    default:
      break;
  }

 display_add (display, gg);

 return(display);
}

gint
display_add(displayd *display, ggobid *gg)
{
  splotd *prev_splot = gg->current_splot;

  GGobi_widget_set(display->window, gg, true);

  display_set_current (display, gg);
  gg->displays = g_list_append (gg->displays, (gpointer) display);

    /* If the tree of displays is active, add this to it. */
  display_add_tree(display, -1, gg->app.display_tree.tree, gg);

  gg->current_splot = (splotd *)
    g_list_nth_data (gg->current_display->splots, 0);
  splot_set_current (gg->current_splot, on, gg);

  /*
   * The current display types start without signal handlers, but
   * I may need to add handlers later for some unforeseen display.
  */
  mode_set (gg->current_display->cpanel.mode, gg);  /* don't activate */

  /*
   * Make sure the border for the previous plot is turned off
  */
  prev_splot->redraw_style = QUICK;
  gtk_widget_queue_draw (prev_splot->da);

 return(g_list_length(gg->displays));
}


/*
 * Remove display from the linked list of displays.  Reset
 * current_display and current_splot if necessary.
*/ 
void
display_free (displayd* display, gboolean force, ggobid *gg) {
  GList *l;
  splotd *sp = NULL;
  extern int num_ggobis;
  int count;
/*
 * If the current splot belongs to this display, turn off its
 * event handlers before freeing all the splots belonging to this
 * display.
*/
  displayd *d = (displayd *) gg->current_splot->displayptr;
  if (d == display) {
     sp_event_handlers_toggle (gg->current_splot, off);
  }

  if (num_ggobis > 1 || force || g_list_length (gg->displays) > 1) {

    /* If the display tree is active, remove the corresponding
       entry.
     */
    tree_display_entry_remove(display, gg->app.display_tree.tree, gg); 

    g_list_remove (gg->displays, display);

    if (display == gg->current_display && (g_list_length (gg->displays) > 1)) {
      display_set_current (g_list_nth_data (gg->displays, 0), gg);
      gg->current_splot = (splotd *)
        g_list_nth_data (gg->current_display->splots, 0);
      splot_set_current (gg->current_splot, on, gg);

      sp = gg->current_splot;
      if(sp != NULL) {
        sp->redraw_style = QUICK;
        gtk_widget_queue_draw (sp->da);
      }
    }

    count = g_list_length(display->splots);
    for (l=display->splots; count > 0 && l; l=l->next, count--) {
      sp = (splotd *) l->data;
      splot_free (sp, display, gg);
    }
    gtk_widget_destroy (display->window);
    g_free (display);
  } else
    quick_message ("Sorry, you can't delete the only display\n", false);
}

void
display_free_all (ggobid *gg) {
  GList *dlist;
  displayd *display;
  int count = g_list_length(gg->displays);

  /* Have to count down rather than rely on dlist being non-null.
     This is because when we remove the last entry, we get garbage
     not a null value.
   */
  for (dlist = gg->displays; count > 0 && dlist ; dlist = dlist->next, count--) {
    /*    display = (displayd *) dlist->data; */
    display = (displayd*) g_list_nth_data(gg->displays,count-1);
     /* Perhaps this force should be true to get the final one.
        It will work now if there is more than one ggobi instance
        running.
      */
    display_free (display, false, gg); 
  }
}

void
display_set_current (displayd *new_display, ggobid *gg) 
{
  if(new_display == NULL)
   return;

  gtk_accel_group_unlock (gg->main_accel_group);

  if (gg->firsttime == false) {

    switch (gg->current_display->displaytype) {
      case scatterplot:
        gtk_window_set_title (GTK_WINDOW (gg->current_display->window),
          (gg->current_display->missing_p) ? "scatterplot display (missings)" :
                                            "scatterplot display"); 
        submenu_destroy (gg->mode_item);
        break;

      case scatmat:
        gtk_window_set_title (GTK_WINDOW (gg->current_display->window),
          (gg->current_display->missing_p) ? "scatterplot matrix (missings)" :
                                         "scatterplot matrix"); 
        submenu_destroy (gg->mode_item);
        break;

      case parcoords:
        gtk_window_set_title (GTK_WINDOW (gg->current_display->window),
                              "parallel coordinates display");
        submenu_destroy (gg->mode_item);
        break;
    }
  }

  switch (new_display->displaytype) {
    case scatterplot:
      gtk_window_set_title (GTK_WINDOW (new_display->window),
        (new_display->missing_p) ?
          "*** scatterplot display (missings) *** " :
          "*** scatterplot display ***"); 

      scatterplot_main_menus_make (gg->main_accel_group, mode_set_cb, gg, true);
      gg->mode_item = submenu_make ("_View", 'V', gg->main_accel_group);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_item),
                                 gg->app.scatterplot_mode_menu); 
      submenu_insert (gg->mode_item, gg->main_menubar, 2);
      break;

    case scatmat:
      gtk_window_set_title (GTK_WINDOW (new_display->window),
        (new_display->missing_p) ?
          "*** scatterplot matrix (missings) *** " :
          "*** scatterplot matrix ***"); 

      scatmat_main_menus_make (gg->main_accel_group, mode_set_cb, gg, true);
      gg->mode_item = submenu_make ("_View", 'V', gg->main_accel_group);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_item),
                                 gg->app.scatmat_mode_menu); 
      submenu_insert (gg->mode_item, gg->main_menubar, 2);
      break;

    case parcoords:
      gtk_window_set_title (GTK_WINDOW (new_display->window),
                            "*** parallel coordinates display ***");

      parcoords_main_menus_make (gg->main_accel_group, mode_set_cb, gg, true);
      gg->mode_item = submenu_make ("_View", 'V', gg->main_accel_group);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_item),
                                 gg->app.parcoords_mode_menu); 
      submenu_insert (gg->mode_item, gg->main_menubar, 2);
      break;
  }

  gg->current_display = new_display;
  cpanel_set (gg->current_display, gg);

  varpanel_refresh (gg);

  gtk_accel_group_lock (gg->main_accel_group);
  gg->firsttime = false;
}

/* Some of these will probably be folded together eventually */

/*
 * replot all splots in display -- except splot
 * if splot is NULL, replot everything
*/
void
displays_plot (splotd *splot, ggobid *gg) {
  GList *dlist, *slist;
  displayd *display;
  splotd *sp;

  for (dlist = gg->displays; dlist; dlist = dlist->next) {
    display = (displayd *) dlist->data;
    for (slist = display->splots; slist; slist = slist->next) {
      sp = (splotd *) slist->data;
      if (sp != NULL) {
        if (splot == NULL || sp != splot) {
          splot_redraw (sp, FULL, gg);
        }
      }
    }
  }
}


/*-- reproject and replot all splots in display --*/
void
display_tailpipe (displayd *display, ggobid *gg) {
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

    if (display->displaytype == scatterplot)
      ruler_ranges_set (display, sp, gg);

    splot_redraw (sp, FULL, gg);
    splist = splist->next;
  }
}

/*-- Reproject and plot all plots in all displays: modulo missingness --*/
void
displays_tailpipe (gint which, ggobid *gg) {
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
      display_tailpipe (display, gg);
  }
}

void
display_window_init(displayd *display, int width, ggobid *gg)
{
  display->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (display->window),
                       "displayd",
                       (gpointer) display);
  gtk_window_set_policy (GTK_WINDOW (display->window), true, true, false);
  gtk_container_set_border_width (GTK_CONTAINER (display->window), width);
  gtk_signal_connect (GTK_OBJECT (display->window), "delete_event",
                      GTK_SIGNAL_FUNC (display_delete_cb), (gpointer) display);

  GGobi_widget_set(GTK_WIDGET(display->window), gg, true);
}
