
/* display.c routines */
void
display_new (gpointer   cbd,
             guint      action,
             GtkWidget  *widget)
{
  displayd *display;
  gint mode = current_display->cpanel.mode;

  /*
   * Turn off event handlers, remove submenus, and redraw the
   * previous plot without a border.
  */
  splot_set_current (current_splot, off);
  splot_toggle_border (current_splot, off);

  switch (action) {

    case scatterplot:
      display = scatterplot_new ();
      break;

    case scatmat:
      display = scatmat_new ();
      break;

    case parcoords:
      display = parcoords_new ();
      break;
  }

  display_set_current (display);
  displays = g_list_append (displays, (gpointer) display);

  current_splot = (splotd *) g_list_nth_data (current_display->splots, 0);
  splot_set_current (current_splot, on);
  /* Does it start out with the correct border? */

  /*
   * The current display types start without signal handlers, but
   * I may need to add handlers later for some unforeseen display.
  */
  mode_set (current_display->cpanel.mode);  /* don't activate */
}

/*
 * Remove display from the linked list of displays.  Reset
 * current_display and current_splot if necessary.
*/ 
void
display_free (displayd* display) {
  GList *l;
  splotd *sp;

  if (g_list_length (displays) > 1) {

    g_list_remove (displays, display);
    if (current_display == display) {
      display_set_current (g_list_nth_data (displays, 0));
      current_splot = (splotd *) g_list_nth_data (current_display->splots, 0);
      splot_set_current (current_splot, on);
    }

    for (l=display->splots; l; l=l->next) {
      sp = (splotd *) l->data;
      splot_free (sp);
    }
    gtk_widget_destroy (display->window);
    g_free (display);
  } else
    quick_message ("Sorry, you can't delete the only display\n");
}

void
display_set_current (displayd *new_display) {
  extern GtkWidget *menubar;
  extern GtkAccelGroup *main_accel_group;

  static GtkWidget *mode_item = NULL;
  extern GtkWidget *scatterplot_mode_menu;
  extern GtkWidget *scatmat_mode_menu;
  extern GtkWidget *parcoords_mode_menu;

  static gboolean firsttime = true;
  static gboolean firsttime_mode = true;

  gtk_accel_group_unlock (main_accel_group);

  if (!firsttime) {

/*    activate_mode (current_splot, current_display->cpanel.mode, off);*/

    switch (current_display->displaytype) {
      case scatterplot:
        gtk_window_set_title (GTK_WINDOW (current_display->window),
                              "scatterplot display");
        destroy_submenu (mode_item);
        break;

      case scatmat:
        gtk_window_set_title (GTK_WINDOW (current_display->window),
                              "scatterplot matrix");
        destroy_submenu (mode_item);
        break;

      case parcoords:
        gtk_window_set_title (GTK_WINDOW (current_display->window),
                              "parallel coordinates display");
        destroy_submenu (mode_item);
        break;
    }
  }

  switch (new_display->displaytype) {
    case scatterplot:
      gtk_window_set_title (GTK_WINDOW (new_display->window),
                            "*** scatterplot display ***");

      make_scatterplot_menus (main_accel_group, set_mode_cb);
      mode_item = make_submenu ("_Mode", 'M', main_accel_group);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (mode_item),
                                 scatterplot_mode_menu); 
      insert_submenu (mode_item, menubar, 2);
      if (firsttime_mode) {
        firsttime_mode = false;
      }
      break;

    case scatmat:
      gtk_window_set_title (GTK_WINDOW (new_display->window),
                            "*** scatterplot matrix ***");

      make_scatmat_menus (main_accel_group, set_mode_cb);
      mode_item = make_submenu ("_Mode", 'M', main_accel_group);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (mode_item),
                                 scatmat_mode_menu); 
      insert_submenu (mode_item, menubar, 2);
      if (firsttime_mode) {
        firsttime_mode = false;
      }
      break;

    case parcoords:
      gtk_window_set_title (GTK_WINDOW (new_display->window),
                            "*** parallel coordinates display ***");

      make_parcoords_menus (main_accel_group, set_mode_cb);
      mode_item = make_submenu ("_Mode", 'M', main_accel_group);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (mode_item),
                                 parcoords_mode_menu); 
      insert_submenu (mode_item, menubar, 2);
      if (firsttime_mode) {
        firsttime_mode = false;
      }
      break;
  }

  current_display = new_display;
  show_control_panel (&current_display->cpanel);

/*  activate_mode (current_splot, current_display->cpanel.mode, on);*/

  gtk_accel_group_lock (main_accel_group);
  firsttime = false;
}


/* end of display.c routines */

/* splot.c routines */

void
splot_toggle_border (splotd *sp, gboolean state) {

  if (sp != null && sp->da != null && sp->da->window != null) {
    if (state)
      splot_add_border (sp);
    else
      /*
       * Copy the backing pixmap to the window for the
       * previous splot: this is solely to eliminate the
       * border.
      */
      gdk_draw_pixmap (sp->da->window, plot_GC, sp->pixmap,
                       0, 0, 0, 0,
                       da->allocation.width, da->allocation.height);
    }
  }
}

void
splot_set_current (splotd *sp, gboolean state) {
/*
 * Turn on or off the event handlers in sp
*/
  if (sp != null) {
    displayd *display = (displayd *) sp->displayptr;
    cpaneld *cpanel = &display->cpanel;

    mode_event_handlers_toggle (current_splot, cpanel->mode, state);
    mode_submenus_activate (current_splot, mode, state)
  }
}


static gint
splot_set_current_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  displayd *display = (displayd *) sp->displayptr; 

  if (sp != current_splot) {

    splot_set_current (current_splot, off);
    splot_toggle_border (current_splot, off);

    if (current_display != display)
      display_set_current (display);  /* old one off, new one on */

    current_splot = sp;
    splot_toggle_border (sp, on);
    splot_set_current (sp, on);
  }

  return false;  /* so that other button press handlers also get the event */
}
