/* main_ui.c */

#include <gtk/gtk.h>
#include <strings.h>

#include "vars.h"

/* external functions */
extern void get_main_menu (GtkItemFactoryEntry[], gint, GtkAccelGroup *,
                           GtkWidget  *, GtkWidget **, gpointer);
extern void display_new ();
extern void display_reproject (displayd *);
extern void display_toggle_brush_handlers (gboolean);
extern void display_toggle_ctour_handlers (gboolean);
extern void display_toggle_gtour_handlers (gboolean);
extern void identify_toggle_splot_handlers (splotd *, gboolean);
extern void display_toggle_lineedit_handlers (gboolean);
extern void display_toggle_movepts_handlers (gboolean);
extern void display_toggle_rotation_handlers (gboolean);
extern void display_toggle_scale_handlers (gboolean);
extern void init_varpanel_geometry (gboolean, gint, gint);
extern void make_brush_menus ();
extern void cpanel_brush_make ();
extern void cpanel_ctour_make ();
extern void make_gtour_menus ();
extern void make_scatmat_menus ();
extern void cpanel_gtour_make ();
extern void make_identify_menus ();
extern void cpanel_identify_make ();
extern void cpanel_lineedit_make ();
extern void cpanel_movepts_make ();
extern void cpanel_p1dplot_make ();
extern void make_rotation_menus ();
extern void cpanel_rotation_make ();
extern void cpanel_parcoords_make ();
extern void cpanel_scatmat_make ();
extern void make_scale_menus ();
extern void cpanel_scale_make ();
extern void make_varpanel ();
extern void cpanel_xyplot_make ();
extern void open_jitter_popup ();
extern void open_smooth_popup ();
extern void open_subset_popup ();
extern void open_transform_popup ();
extern void set_mode (gint);
extern void brush_options_cb (gpointer, guint, GtkCheckMenuItem *);
extern GtkWidget * make_submenu (gchar *, gint, GtkAccelGroup *);
extern void insert_submenu (GtkWidget *, GtkWidget *, gint);
extern void destroy_submenu (GtkWidget *);
extern void scatterplot_show_rulers (displayd *, gint);
/* */

static gint mode = XYPLOT, prev_mode = XYPLOT;
static gint projection = XYPLOT, prev_projection = XYPLOT;

GtkWidget *menubar;
GtkAccelGroup *main_accel_group;

static GtkWidget *mode_frame;
static char *mode_name[] = {
  "1DPlot",
  "XYPlot",
  "Rotation",
  "Grand Tour",
  "Correlation Tour",
  "Scale",
  "Brush",
  "Identify",
  "Edit Lines",
  "Move Points",

  "Scatmat",
  "Parcoords",
};

/* w is the toplevel window */
static gint
window_map_cb (GtkWidget *w, GdkEventConfigure *event)
{
  /* remove the xyplot panel */
  gtk_widget_ref (control_panel[mode]);
  gtk_container_remove (GTK_CONTAINER (mode_frame), control_panel[mode]);

  /* add the cotour panel, currently the largest, and resize */
  gtk_container_add (GTK_CONTAINER (mode_frame), control_panel[COTOUR]);
  gtk_container_check_resize (GTK_CONTAINER (w));

  /* remove the cotour panel */
  gtk_widget_ref (control_panel[COTOUR]);
  gtk_container_remove (GTK_CONTAINER (mode_frame), control_panel[COTOUR]);

  /* restore the xyplot panel */
  gtk_container_add (GTK_CONTAINER (mode_frame), control_panel[mode]);

  return false;
}

void
make_control_panels () {

/* rename these:  cpanel_blahblah_make */
  cpanel_p1dplot_make ();
  cpanel_xyplot_make ();
  cpanel_rotation_make ();
  cpanel_gtour_make ();
  cpanel_ctour_make ();

  cpanel_brush_make ();
  cpanel_scale_make ();
  cpanel_identify_make ();
  cpanel_lineedit_make ();
  cpanel_movepts_make ();

  cpanel_parcoords_make ();
  cpanel_scatmat_make ();
}


void
toggle_options (gpointer data, guint action, GtkCheckMenuItem *w) {

  g_printerr ("action = %d\n", action);

  switch (action) {

    case 0:
      if (w->active) gtk_tooltips_enable (xg.tips);
      else gtk_tooltips_disable (xg.tips);
      break;

    case 1:
      if (mode_frame != NULL) {  /* so it doesn't complain on startup */
        if (w->active)
          gtk_widget_hide (mode_frame);
        else
          gtk_widget_show (mode_frame);
      }
      break;

    case 2:
      if (current_display != null) {

        if (current_display->displaytype == scatterplot) {
          if (current_display->hrule != NULL &&
              current_display->cpanel.projection == XYPLOT)
          {
            if (w->active) {
              gtk_widget_show (current_display->hrule);
              gtk_widget_show (current_display->vrule);
            }
            else {
              gtk_widget_hide (current_display->hrule);
              gtk_widget_hide (current_display->vrule);
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


/* 
 * This does two things:  it resets the main menubar for each
 * modetype, and it attaches the appropriate event-handlers for
 * events in the display window.
*/
void
activate_mode (splotd *sp, gint m, gboolean state)
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
        break;

      case P1PLOT:
        break;

      case XYPLOT:
        break;

      case ROTATE:
        display_toggle_rotation_handlers (off);
        destroy_submenu (io_item);
        break;

      case GRTOUR:
        display_toggle_gtour_handlers (off);
        destroy_submenu (io_item);
        break;

      case COTOUR:
        display_toggle_ctour_handlers (off);
        break;

      case SCALE:
        display_toggle_scale_handlers (off);
        destroy_submenu (reset_item);
        break;

      case BRUSH:
        display_toggle_brush_handlers (off);
        destroy_submenu (reset_item);
        destroy_submenu (link_item);
        break;

      case IDENT:
        identify_toggle_plot_handlers (sp, off);
        destroy_submenu (link_item);
        break;

      case LINEED:
        display_toggle_lineedit_handlers (off);
        break;

      case MOVEPTS:
        display_toggle_movepts_handlers (off);
        break;
    }
  } else if (state == on) {

    switch (m) {
      case PCPLOT:
        break;

      case P1PLOT:
      case XYPLOT:
        break;

      case ROTATE:
        display_toggle_rotation_handlers (on);
        make_rotation_menus ();

        io_item = make_submenu ("_I/O", 'I', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (io_item), rotation_io_menu); 
        if (firsttime_io) {
          insert_submenu (io_item, menubar, -1);
          firsttime_io = false;
        }
        break;

      case GRTOUR:
        display_toggle_gtour_handlers (on);
        make_gtour_menus ();

        io_item = make_submenu ("_I/O", 'I', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (io_item), gtour_io_menu); 
        if (firsttime_io) {
          insert_submenu (io_item, menubar, -1);
          firsttime_io = false;
        }
        break;

      case COTOUR:
        display_toggle_ctour_handlers (on);
        break;

      case SCALE :
        display_toggle_scale_handlers (on);
        make_scale_menus ();

        reset_item = make_submenu ("_Reset", 'R', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (reset_item),
                                   scale_reset_menu); 
        if (firsttime_reset) {
          insert_submenu (reset_item, menubar, -1);
          firsttime_reset = false;
        }
        break;

      case BRUSH :
        display_toggle_brush_handlers (on);
        make_brush_menus ();

        reset_item = make_submenu ("_Reset", 'R', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (reset_item),
                                   brush_reset_menu); 
        if (firsttime_reset) {
          insert_submenu (reset_item, menubar, -1);
          firsttime_reset = false;
        }

        link_item = make_submenu ("_Link", 'L', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (link_item),
                                   brush_link_menu); 
        if (firsttime_link) {
          insert_submenu (link_item, menubar, -1);
          firsttime_link = false;
        }
        break;

      case IDENT:
        identify_toggle_splot_handlers (sp, on);
        make_identify_menus ();

        link_item = make_submenu ("_Link", 'L', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (link_item),
                                   identify_link_menu); 
        if (firsttime_link) {
          insert_submenu (link_item, menubar, -1);
          firsttime_link = false;
        }
        break;

      case LINEED:
        display_toggle_lineedit_handlers (on);
        break;

      case MOVEPTS:
        display_toggle_movepts_handlers (on);
        break;
    }
  }
}


void 
set_mode (gint m) {
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
/*    gtk_widget_show_all (control_panel[mode]);*/

/*
g_printerr ("width %d height %d\n",
mode_frame->requisition.width, mode_frame->requisition.height);
*/
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
set_mode_cb (gpointer   cbd,
             guint      action,
             GtkWidget  *widget)
{
  if (current_display != null && current_splot != null) {
    activate_mode (current_splot, mode, off);

    current_display->cpanel.mode = action;
    set_mode (action);

    activate_mode (current_splot, action, on);
    display_reproject (current_display);
  }
}

static GtkItemFactoryEntry menu_items[] = {
  { "/_File",        NULL,     NULL,          0, "<Branch>" },
  { "/File/Read",    NULL,     NULL,          0 },
  { "/File/Save (extend file set)",   
                      NULL,     NULL,          0 },
  { "/File/Save (new file set)",   
                      NULL,     NULL,          0 },
  { "/File/Print",   NULL,     NULL,          0 },
  { "/File/sep",     NULL,     NULL,          0, "<Separator>" },
  { "/File/Quit",    "<ctrl>Q",     gtk_main_quit, 0 },

  { "/_Display",                            NULL,   
    NULL,        0,    "<Branch>" },
  { "/Display/New scatterplot",               "",
    display_new, scatterplot },
  { "/Display/New scatterplot matrix",        "",
    display_new, scatmat },
  { "/Display/New parallel coordinates plot", "",
    display_new, parcoords },

  { "/_Tools",        NULL,         NULL, 0, "<Branch>" },
  { "/Tools/Hide or exclude ...", 
                      NULL,         NULL, 0, NULL },
  { "/Tools/Sample or subset ...", 
                      NULL,         open_subset_popup,    0, NULL },
  { "/Tools/sep",     NULL, NULL, 0, "<Separator>" },
  { "/Tools/Smooth ...", 
                      NULL,         open_smooth_popup,    0, NULL },
  { "/Tools/Jitter ...", 
                      NULL,         open_jitter_popup,    0, NULL },
  { "/Tools/sep",              NULL,    NULL,             0, "<Separator>" },
  { "/Tools/Clone XGobi ...", 
                      NULL,         NULL, 0, NULL },
  { "/Tools/Scatterplot matrix ...", 
                      NULL,         NULL, 0, NULL },
  { "/Tools/sep",              NULL,    NULL,          0, "<Separator>" },
  { "/Tools/Variable transformation ...", 
                      NULL,         open_transform_popup, 0, NULL },
  { "/Tools/sep",              NULL,    NULL,          0, "<Separator>" },
  { "/Tools/Launch missing values XGobi ...", 
                      NULL,         NULL, 0, NULL },
  { "/Tools/Impute missing values ...", 
                      NULL,         NULL, 0, NULL },

  { "/_Options",      NULL,         NULL, 0, "<Branch>" },
  { "/Options/Show tooltips",  
                      "<ctrl>t",         toggle_options, 0, "<CheckItem>" },
  { "/Options/Show control _panel",  
                      "<ctrl>p" ,        toggle_options, 1, "<CheckItem>" },
  { "/Options/sep",   NULL,     NULL,          0, "<Separator>" },

  { "/Options/Brushing",  
                      NULL ,        NULL, 0, "<Branch>" },
  { "/Options/Brushing/Brush jumps to cursor",  
                      NULL ,        brush_options_cb, 0, "<CheckItem>" },
  { "/Options/Brushing/Update linked brushing continuously",  
                      NULL ,        brush_options_cb, 1, "<CheckItem>" },

  { "/_Help",         NULL,         NULL, 0, "<LastBranch>" },
  { "/Help/About XGobi",  
                      NULL,         NULL, 0, NULL },
  { "/Help/About help ...",  
                      NULL,         NULL, 0, NULL },
};


void make_ui () {
  GtkWidget *window;
  GtkWidget *hbox, *vbox;
  GList *items, *subitems;
  GtkWidget *item, *submenu;
  gchar *name;

  xg.tips = gtk_tooltips_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_policy (GTK_WINDOW (window), true, true, false);

  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_exit), NULL);
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (gtk_exit), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (false, 1);
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  main_accel_group = gtk_accel_group_new ();
  get_main_menu (menu_items, sizeof (menu_items) / sizeof (menu_items[0]),
                 main_accel_group, window, &menubar, (gpointer) null);
  gtk_box_pack_start (GTK_BOX (vbox), menubar, false, true, 0);

  gtk_accel_group_lock (main_accel_group);

/*
 * Step through the option menu, setting the values of toggle items
 * as appropriate.
*/

  items = gtk_container_children (GTK_CONTAINER (menubar));
  while (items) {
    item = items->data;
    gtk_label_get (GTK_LABEL (GTK_MENU_ITEM (item)->item.bin.child ), &name);
    if (strcmp (name, "Options") == 0) {
      submenu = GTK_MENU_ITEM (item)->submenu;
      subitems = gtk_container_children (GTK_CONTAINER (submenu));
      while (subitems) {
        item = subitems->data;

        if (GTK_IS_CHECK_MENU_ITEM (item)) {
          gtk_label_get (GTK_LABEL (GTK_MENU_ITEM (item)->item.bin.child),
            &name);
          g_printerr ("%s\n", name);

          if (g_strcasecmp (name, "show tooltips") == 0)
            gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), true);
          else if (g_strcasecmp (name, "show mode panel") == 0)
            gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), true);
  
        } else {
          /* If there are to be submenus, I have to dig them out as well. */
        }
        subitems = subitems->next;
      }
      break;  /* Finished with options menu */
    }

    items = items->next;
  }
/* */

  hbox = gtk_hbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, true, true, 0);

/*
 * Create a frame to hold the mode panels, set its label
 * and contents, using the default mode for the default display.
*/
  mode_frame = gtk_frame_new (mode_name[mode]);
  gtk_box_pack_start (GTK_BOX (hbox), mode_frame, false, false, 3);
  gtk_container_set_border_width (GTK_CONTAINER (mode_frame), 3);
  gtk_frame_set_shadow_type (GTK_FRAME (mode_frame), GTK_SHADOW_IN);

  make_control_panels ();
  gtk_container_add (GTK_CONTAINER (mode_frame), control_panel[mode]);

  /*
   * Variable selection panel
  */
  init_varpanel_geometry (true, 0, 0);
  make_varpanel (hbox);

  gtk_widget_show_all (hbox);

  gtk_signal_connect (GTK_OBJECT (window),
                      "map_event",
                      (GtkSignalFunc) window_map_cb,
                      (gpointer) null);

  gtk_widget_show_all (window);
}
