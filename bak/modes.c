/* mode routines:  fold back into main_ui.c */

void
mode_event_handlers_toggle (splot *sp, gint m, gboolean state) {

  switch (m) {
    case PCPLOT:
    case P1PLOT:
    case XYPLOT:
      break;

    case ROTATE:
      rotation_event_handlers_toggle (sp, state);
      break;

    case GRTOUR:
      gtour_event_handlers_toggle (sp, state);
      break;

    case COTOUR:
      ctour_event_handlers_toggle (sp, state);
      break;

    case SCALE:
      scale_event_handlers_toggle (sp, state);
      break;

    case BRUSH:
      brush_event_handlers_toggle (sp, state);
      break;

    case IDENT:
      identify_event_handlers_toggle (sp, state);
      break;

    case LINEED:
      lineedit_event_handlers_toggle (sp, state);
      break;

    case MOVEPTS:
      movepts_event_handlers_toggle (sp, state);
      break;
  }
}

void
mode_submenus_activate (splotd *sp, gint m, gboolean state)
{
  extern GtkWidget *scale_reset_menu;
  extern GtkWidget *brush_reset_menu, *brush_link_menu;
  extern GtkWidget *identify_link_menu;
  extern GtkWidget *rotation_io_menu, *gtour_io_menu;

  static GtkWidget *reset_item = NULL;
  static GtkWidget *link_item = NULL;
  static GtkWidget *io_item = NULL;

  static gboolean firsttime_reset = true;
  static gboolean firsttime_link = true;
  static gboolean firsttime_io = true;

  if (state == off) {

    switch (m) {
      case PCPLOT:
      case P1PLOT:
      case XYPLOT:
      case LINEED:
      case MOVEPTS:
      case COTOUR:
        break;

      case ROTATE:
        submenu_destroy (io_item);
        break;

      case GRTOUR:
        submenu_destroy (io_item);
        break;

      case SCALE:
        submenu_destroy (reset_item);
        break;

      case BRUSH:
        submenu_destroy (reset_item);
        submenu_destroy (link_item);
        break;

      case IDENT:
        submenu_destroy (link_item);
        break;

    }
  } else if (state == on) {

    switch (m) {
      case PCPLOT:
      case P1PLOT:
      case XYPLOT:
      case COTOUR:
      case LINEED:
      case MOVEPTS:
        break;

      case ROTATE:
        rotation_menus_make ();

        io_item = make_submenu ("_I/O", 'I', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (io_item), rotation_io_menu); 
        if (firsttime_io) {
          submenu_insert (io_item, menubar, -1);
          firsttime_io = false;
        }
        break;

      case GRTOUR:
        gtour_menus_make ();

        io_item = make_submenu ("_I/O", 'I', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (io_item), gtour_io_menu); 
        if (firsttime_io) {
          submenu_insert (io_item, menubar, -1);
          firsttime_io = false;
        }
        break;


      case SCALE :
        scale_menus_make ();

        reset_item = make_submenu ("_Reset", 'R', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (reset_item),
                                   scale_reset_menu); 
        if (firsttime_reset) {
          submenu_insert (reset_item, menubar, -1);
          firsttime_reset = false;
        }
        break;

      case BRUSH :
        brush_menus_make ();

        reset_item = make_submenu ("_Reset", 'R', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (reset_item),
                                   brush_reset_menu); 
        if (firsttime_reset) {
          submenu_insert (reset_item, menubar, -1);
          firsttime_reset = false;
        }

        link_item = make_submenu ("_Link", 'L', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (link_item),
                                   brush_link_menu); 
        if (firsttime_link) {
          submenu_insert (link_item, menubar, -1);
          firsttime_link = false;
        }
        break;

      case IDENT:
        identify_menus_make ();

        link_item = make_submenu ("_Link", 'L', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (link_item),
                                   identify_link_menu); 
        if (firsttime_link) {
          submenu_insert (link_item, menubar, -1);
          firsttime_link = false;
        }
        break;

    }
  }
}

/*
 * Set the mode for the current display
*/
void 
mode_set (gint m) {
  mode = m;

  if (mode <= COTOUR)  projection = mode;

  /*
   * The projection type is one of P1PLOT, XYPLOT, ROTATE,
   * GRTOUR or COTOUR.  It only changes if another projection
   * type is selected.
  */
  if (mode <= COTOUR)  current_display->cpanel.projection = mode;
  
  if (mode != prev_mode) {

    /* Add a reference to the widget so it isn't destroyed */
    gtk_widget_ref (control_panel[prev_mode]);
    gtk_container_remove (GTK_CONTAINER (mode_frame),
                          control_panel[prev_mode]);
  
    gtk_frame_set_label (GTK_FRAME (mode_frame), mode_name[mode]);
    gtk_container_add (GTK_CONTAINER (mode_frame), control_panel[mode]);
  }

  
  if (projection != prev_projection &&
      current_display->displaytype == scatterplot) 
  {
    scatterplot_show_rulers (current_display, projection);
  }

  prev_mode = mode;
  prev_projection = projection;
}


void
mode_set_cb (gpointer   cbd,
             guint      action,
             GtkWidget  *widget)
{
  if (current_display != null && current_splot != null) {

/*  activate_mode (current_splot, mode, off);*/
    mode_event_handlers_toggle (current_splot, mode, off);
    mode_submenus_activate (current_splot, mode, off)

    current_display->cpanel.mode = action;
    mode_set (action);  /* mode = action */

/*  activate_mode (current_splot, action, on);*/
    mode_event_handlers_toggle (current_splot, mode, on);
    mode_submenus_activate (current_splot, mode, on)

    display_reproject (current_display);
  }
}
