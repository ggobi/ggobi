/* main_ui.c */

#include <strings.h>
#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"
#include "display_tree.h"


const char *const GGOBI(ModeNames)[] = {
  "1D Plot",
  "XYPlot",
  "Rotation",
  "2D Tour",
  "Correlation Tour",
  "Scale",
  "Brush",
  "Identify",
  "Edit Lines",
  "Move Points",

  "Scatmat",
  "Parcoords",
};

const char *const *mode_name = GGOBI(ModeNames);

/* w is the toplevel window */
static gint
size_allocate_cb (GtkWidget *w, GdkEvent *event, ggobid *gg)
{
  static gboolean initd = false;

  if (!initd ) {  /*-- only do this once --*/

    /*-- Use the largest control panel, which is currently COTOUR --*/
    GtkWidget *largest_panel = gg->control_panel[COTOUR];
    GtkWidget *mode_panel = gg->control_panel[gg->mode];

    /* remove the xyplot panel */
    gtk_widget_ref (mode_panel);
    gtk_container_remove (GTK_CONTAINER (gg->mode_frame), mode_panel);

    /* add the largest panel and resize */
    gtk_container_add (GTK_CONTAINER (gg->mode_frame), largest_panel);
    gtk_container_check_resize (GTK_CONTAINER (w));

    /* remove the largest panel and restore the xyplot panel */
    gtk_widget_ref (largest_panel);
    gtk_container_remove (GTK_CONTAINER (gg->mode_frame), largest_panel);
    gtk_container_add (GTK_CONTAINER (gg->mode_frame), mode_panel);

    /*-- widen the variable selection panel --*/
    varpanel_size_init (largest_panel->requisition.height, gg);

    initd = true;
  }

  return false;
}

void
make_control_panels (ggobid *gg) {

  cpanel_p1dplot_make (gg);
  cpanel_xyplot_make (gg);
  cpanel_rotation_make (gg);
  cpanel_tour2d_make (gg);
  cpanel_ctour_make (gg);

  cpanel_brush_make (gg);
  cpanel_scale_make (gg);
  cpanel_identify_make (gg);
  cpanel_lineedit_make (gg);
  cpanel_movepts_make (gg);

  cpanel_parcoords_make (gg);
  cpanel_scatmat_make (gg);
}


void
main_display_options_cb (ggobid *gg, guint action, GtkCheckMenuItem *w) 
{
  if (gg->mode_frame == NULL)  /* so it isn't executed on startup */
    return;
  else
    g_printerr ("(main_display_options_cb) action = %d\n", action);

  switch (action) {

    case 0:
      if (w->active) gtk_tooltips_enable (gg->tips);
      else gtk_tooltips_disable (gg->tips);
      break;

    case 1:
      if (w->active)
        gtk_widget_hide (gg->mode_frame);
      else
        gtk_widget_show (gg->mode_frame);
      break;

    case 2:
      if (gg->current_display != NULL) {
        displayd *display = gg->current_display;

        if (display->displaytype == scatterplot) {
          if (display->hrule != NULL &&
              display->cpanel.projection == XYPLOT)
          {
            if (w->active) {
              gtk_widget_show (display->hrule);
              gtk_widget_show (display->vrule);
            }
            else {
              gtk_widget_hide (display->hrule);
              gtk_widget_hide (display->vrule);
            }
          }
        }

      }
      break;

    case 3:
      g_printerr ("toggle gridlines\n");
      break;
    case 4:
      g_printerr ("toggle centering axes\n");
      break;
    case 5:
      g_printerr ("toggle plotting points\n");
      break;
    case 6:
      g_printerr ("toggle plotting lines\n");
      break;
    case 7:
      g_printerr ("toggle plotting directed lines\n");
      break;
  }
}


void
mode_submenus_activate (splotd *sp, gint m, gboolean state, ggobid *gg)
{

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
        submenu_destroy (gg->mode_menu.io_item);
        break;

      case TOUR2D:
        submenu_destroy (gg->mode_menu.io_item);
        break;

      case SCALE:
        submenu_destroy (gg->mode_menu.reset_item);
        break;

      case BRUSH:
        submenu_destroy (gg->mode_menu.reset_item);
        submenu_destroy (gg->mode_menu.link_item);
        break;

      case IDENT:
        submenu_destroy (gg->mode_menu.link_item);
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
        rotation_menus_make (gg);

        gg->mode_menu.io_item = submenu_make ("_I/O", 'I',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.io_item),
          gg->app.rotation_io_menu); 
        if (gg->mode_menu.firsttime_io) {
          submenu_insert (gg->mode_menu.io_item, gg->main_menubar, -1);
          gg->mode_menu.firsttime_io = false;
        }
        break;

      case TOUR2D:
        tour2d_menus_make (gg);

        gg->mode_menu.io_item = submenu_make ("_I/O", 'I',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.io_item),
          gg->tour2d.io_menu); 
        if (gg->mode_menu.firsttime_io) {
          submenu_insert (gg->mode_menu.io_item, gg->main_menubar, -1);
          gg->mode_menu.firsttime_io = false;
        }
        break;

      case SCALE :
        scale_menus_make (gg);

        gg->mode_menu.reset_item = submenu_make ("_Reset", 'R',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.reset_item),
          gg->scale.scale_reset_menu); 
        if (gg->mode_menu.firsttime_reset) {
          submenu_insert (gg->mode_menu.reset_item, gg->main_menubar, -1);
          gg->mode_menu.firsttime_reset = false;
        }
        break;

      case BRUSH :
        brush_menus_make (gg);

        gg->mode_menu.reset_item = submenu_make ("_Reset", 'R',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.reset_item),
          gg->brush.reset_menu);
        if (gg->mode_menu.firsttime_reset) {
          submenu_insert (gg->mode_menu.reset_item, gg->main_menubar, -1);
          gg->mode_menu.firsttime_reset = false;
        }

        gg->mode_menu.link_item = submenu_make ("_Link", 'L',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.link_item),
          gg->brush.link_menu); 
        if (gg->mode_menu.firsttime_link) {
          submenu_insert (gg->mode_menu.link_item, gg->main_menubar, -1);
          gg->mode_menu.firsttime_link = false;
        }

        break;

      case IDENT:
        identify_menus_make (gg);

        gg->mode_menu.link_item = submenu_make ("_Link", 'L',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.link_item),
          gg->identify.link_menu); 
        if (gg->mode_menu.firsttime_link) {
          submenu_insert (gg->mode_menu.link_item, gg->main_menubar, -1);
          gg->mode_menu.firsttime_link = false;
        }
        break;

    }
  }
}

gint
mode_get (ggobid* gg) {
  return gg->mode;
}
gint
projection_get (ggobid* gg) {
  return gg->projection;
}


/*
 * Set the mode for the current display
*/
void 
mode_set (gint m, ggobid *gg) {
  displayd *display = gg->current_display;

  gg->mode = m;
  if (gg->mode != gg->prev_mode) {
    /* Add a reference to the widget so it isn't destroyed */
    gtk_widget_ref (gg->control_panel[gg->prev_mode]);
    gtk_container_remove (GTK_CONTAINER (gg->mode_frame),
                          gg->control_panel[gg->prev_mode]);
  
    gtk_frame_set_label (GTK_FRAME (gg->mode_frame), mode_name[gg->mode]);
    gtk_container_add (GTK_CONTAINER (gg->mode_frame),
      gg->control_panel[gg->mode]);
  }

  /*
   * The projection type is one of P1PLOT, XYPLOT, ROTATE,
   * TOUR2D or COTOUR.  It only changes if another projection
   * type is selected.  (For parcoords and scatmat plots, the
   * value of projection is irrelevant.)
  */
  if (display->displaytype == scatterplot) {
    if (gg->mode <= COTOUR)
      gg->projection = display->cpanel.projection = gg->mode;

    if (gg->projection != gg->prev_projection) {
      scatterplot_show_rulers (display, gg->projection);
      gg->prev_projection = gg->projection;
    }
  }

  gg->prev_mode = gg->mode;

/*  varpanel_refresh ();*/
}

void
mode_activate (splotd *sp, gint m, gboolean state, ggobid *gg) {
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;

  if (state == off) {

    switch (m) {
      case PCPLOT:
      case P1PLOT:
      case XYPLOT:
      case LINEED:
      case MOVEPTS:
      case COTOUR:
      case ROTATE:
        break;

      case TOUR2D:
        tour_func (off, gg);
        break;

      case SCALE:
      case IDENT:
        break;

      case BRUSH:
        break;
    }
  } else if (state == on) {
    switch (m) {
      case PCPLOT:
      case P1PLOT:
      case XYPLOT:
      case LINEED:
      case MOVEPTS:
      case COTOUR:
      case ROTATE:
        break;

      case TOUR2D:
        tour_func (on, gg);
        break;

      case SCALE:
      case IDENT:
        break;

      case BRUSH:
        brush_activate (state, d, gg);
        break;
    }
  }
}

void
mode_set_cb (GtkWidget *widget, gint action)
{
  ggobid *gg = GGobiFromWidget(widget,true);
  GGOBI(full_mode_set)(action, gg);
}

int
GGOBI(full_mode_set)(int action, ggobid *gg)
{
  if (gg->current_display != NULL && gg->current_splot != NULL) {
    splotd *sp = gg->current_splot;
    displayd *display = gg->current_display;

    sp_event_handlers_toggle (sp, off);
    mode_activate (sp, gg->mode, off, gg);
    mode_submenus_activate (sp, gg->mode, off, gg);

    display->cpanel.mode = action;
    mode_set (action, gg);  /* mode = action */

    sp_event_handlers_toggle (sp, on);
    mode_activate (sp, gg->mode, on, gg);
    mode_submenus_activate (sp, gg->mode, on, gg);

    display_tailpipe (display, gg);
    return(action);
  } else
    return(-1);
}

extern void display_write_svg (ggobid *);
static GtkItemFactoryEntry menu_items[] = {
  { "/_File",            NULL,     NULL,             0, "<Branch>" },
  { "/File/Read ...",    NULL,     filename_get_r,   0 },
/*
 *{ "/File/Save (extend file set) ...",   
 *                       NULL,     filename_get,     1 },
*/
  { "/File/Save (new file set) ...",   
                         NULL,     writeall_window_open,     2 },

  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Print",       NULL,     display_write_svg,          0 },
  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Quit",    "<ctrl>Q",     quit_ggobi,  0 },

  { "/_Tools",        NULL,         NULL, 0, "<Branch>" },
  { "/Tools/Variable selection and statistics ...", 
                      NULL,         vartable_open,    0, NULL },
  { "/Tools/Variable transformation ...", 
                      NULL,         transform_window_open, 0, NULL },
  { "/Tools/Variable jittering ...", 
                      NULL,         jitter_window_open,    0, NULL },
  { "/Tools/sep",     NULL, NULL, 0, "<Separator>" },
  { "/Tools/Case hiding and exclusion ...", 
                      NULL,         exclusion_window_open, 0, NULL },
  { "/Tools/Case subsetting and sampling ...", 
                      NULL,         subset_window_open,    0, NULL },
  { "/Tools/sep",     NULL, NULL, 0, "<Separator>" },
  { "/Tools/Smooth ...", 
                      NULL,         smooth_window_open,    0, NULL },

  { "/Tools/Impute missing values ...", 
                      NULL,         impute_window_open,    0, NULL },

/*
 * this code in display.c actually corresponds to the "Window" menu,
 * not this one, which is really just display options.
*/
  { "/_Display",      NULL,         NULL, 0, "<Branch>" },
  { "/Display/Show tooltips",  
                      "<ctrl>t",    main_display_options_cb, 0, "<CheckItem>" },
  { "/Display/Show control _panel",  
                      "<ctrl>p",    main_display_options_cb, 1, "<CheckItem>" },
  { "/Display/sep",   NULL,     NULL,          0, "<Separator>" },

  { "/Display/Brushing",  
                      NULL ,        NULL, 0, "<Branch>" },
  { "/Display/Brushing/Brush jumps to cursor",  
                      NULL ,        brush_options_cb, 0, "<CheckItem>" },
  { "/Display/Brushing/Update linked brushing continuously",  
                      NULL ,        brush_options_cb, 1, "<CheckItem>" },


  {"/_Plots", NULL, NULL, 0, "<Branch>"},
  { "/Plots/Displays",    
          NULL,  show_display_tree, 2},


  { "/_Help",         NULL,         NULL, 0, "<LastBranch>" },
  { "/Help/About GGobi",  
                      NULL,         NULL, 0, NULL },
  { "/Help/About help ...",  
                      NULL,         NULL, 0, NULL },
};


/*
  Wrapper for gtk_main_quit so that we can override this in
  other applications to avoid quitting when the user selects
  the Quit button.
 */
void
quit_ggobi(ggobid *gg, gint action, GtkWidget *w)
{
  gtk_main_quit();
}

void 
make_ui (ggobid *gg) {
  GtkWidget *window;
  GtkWidget *hbox, *vbox;
  GList *items, *subitems;
  GtkWidget *item, *submenu;
  gchar *name;

  gg->tips = gtk_tooltips_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gg->main_window = window;
  GGobi_widget_set (window, gg, true);

  gtk_window_set_policy (GTK_WINDOW (window), true, true, false);

  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                      GTK_SIGNAL_FUNC (ggobi_close), gg);
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (ggobi_close), gg);

  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (false, 1);
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  gg->main_accel_group = gtk_accel_group_new ();
  get_main_menu (menu_items, sizeof (menu_items) / sizeof (menu_items[0]),
                 gg->main_accel_group, window,
                 &gg->main_menubar, (gpointer) gg);

  display_menu_init (gg);

  gtk_box_pack_start (GTK_BOX (vbox), gg->main_menubar, false, true, 0);

  gtk_accel_group_lock (gg->main_accel_group);

/*
 * Step through the option menu, setting the values of toggle items
 * as appropriate.
*/
  items = gtk_container_children (GTK_CONTAINER (gg->main_menubar));
  while (items) {
    item = items->data;
    gtk_label_get (GTK_LABEL (GTK_MENU_ITEM (item)->item.bin.child ), &name);
    if (strcmp (name, "Display") == 0) {
      submenu = GTK_MENU_ITEM (item)->submenu;
      subitems = gtk_container_children (GTK_CONTAINER (submenu));
      while (subitems) {
        item = subitems->data;

        if (GTK_IS_CHECK_MENU_ITEM (item)) {
          gtk_label_get (GTK_LABEL (GTK_MENU_ITEM (item)->item.bin.child),
            &name);
          if (g_strcasecmp (name, "show tooltips") == 0)
            GTK_CHECK_MENU_ITEM (item)->active = true;
          else if (g_strcasecmp (name, "show mode panel") == 0)
            GTK_CHECK_MENU_ITEM (item)->active = true;
  
        } else {
          /* If there are to be submenus, I have to dig them out as well. */
        }
        subitems = subitems->next;
      }
      g_list_free (subitems);
      break;  /* Finished with options menu */
    }

    items = items->next;
  }
  g_list_free (items);
/* */

  hbox = gtk_hbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, true, true, 0);

/*
 * Create a frame to hold the mode panels, set its label
 * and contents, using the default mode for the default display.
*/
  gg->mode_frame = gtk_frame_new (mode_name[gg->mode]);
  gtk_box_pack_start (GTK_BOX (hbox), gg->mode_frame, false, false, 3);
  gtk_container_set_border_width (GTK_CONTAINER (gg->mode_frame), 3);
  gtk_frame_set_shadow_type (GTK_FRAME (gg->mode_frame), GTK_SHADOW_IN);

  make_control_panels (gg);
  gtk_container_add (GTK_CONTAINER (gg->mode_frame),
                     gg->control_panel[gg->mode]);

  /*-- Variable selection panel --*/
  varpanel_make (hbox, gg);

  gtk_widget_show_all (hbox);

  /*-- adjust the size of the main window before it's displayed --*/
  gtk_widget_realize (window);
  gtk_signal_connect (GTK_OBJECT (window),
                      "size_allocate",
                      (GtkSignalFunc) size_allocate_cb,
                      (gpointer) gg);

  gtk_widget_show_all (window);
}


const gchar * const* 
GGOBI(getModeNames)(int *n)
{
  /*  extern const gchar *const* GGOBI(ModeNames); */
  *n = sizeof(GGOBI(ModeNames))/sizeof(GGOBI(ModeNames)[0]);
  return(GGOBI(ModeNames));
}
