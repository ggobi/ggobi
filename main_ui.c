/* main_ui.c */

#include <gtk/gtk.h>
#include <strings.h>

#include "vars.h"
#include "externs.h"


#include "display_tree.h"

static gint mode = XYPLOT, prev_mode = XYPLOT;
static gint projection = XYPLOT, prev_projection = XYPLOT;

GtkWidget *menubar;
GtkAccelGroup *main_accel_group;

static GtkWidget *mode_frame;
static char *mode_name[] = {
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

/* w is the toplevel window */
static gint
size_allocate_cb (GtkWidget *w, GdkEvent *event)
{
  static gboolean initd = false;

  if (!initd ) {  /*-- only do this once --*/

    /*-- Use the largest control panel, which is currently COTOUR --*/
    gint lgst = COTOUR;

    /* remove the xyplot panel */
    gtk_widget_ref (control_panel[mode]);
    gtk_container_remove (GTK_CONTAINER (mode_frame), control_panel[mode]);

    /* add the largest panel and resize */
    gtk_container_add (GTK_CONTAINER (mode_frame), control_panel[lgst]);
    gtk_container_check_resize (GTK_CONTAINER (w));

    /* remove the largest panel and restore the xyplot panel */
    gtk_widget_ref (control_panel[lgst]);
    gtk_container_remove (GTK_CONTAINER (mode_frame), control_panel[lgst]);
    gtk_container_add (GTK_CONTAINER (mode_frame), control_panel[mode]);

    /*-- widen the variable selection panel --*/
    varpanel_size_init (control_panel[lgst]->requisition.height);

    initd = true;
  }

  return false;
}

void
make_control_panels () {

  cpanel_p1dplot_make ();
  cpanel_xyplot_make ();
  cpanel_rotation_make ();
  cpanel_tour2d_make ();
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
main_display_options_cb (gpointer data, guint action, GtkCheckMenuItem *w) {

  if (mode_frame == NULL)  /* so it isn't executed on startup */
    return;
  else
    g_printerr ("(main_display_options_cb) action = %d\n", action);

  switch (action) {

    case 0:
      if (w->active) gtk_tooltips_enable (xg.tips);
      else gtk_tooltips_disable (xg.tips);
      break;

    case 1:
      if (w->active)
        gtk_widget_hide (mode_frame);
      else
        gtk_widget_show (mode_frame);
      break;

    case 2:
      if (current_display != NULL) {

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


void
mode_submenus_activate (splotd *sp, gint m, gboolean state)
{
  extern GtkWidget *scale_reset_menu;
  extern GtkWidget *brush_reset_menu, *brush_link_menu;
  extern GtkWidget *identify_link_menu;
  extern GtkWidget *rotation_io_menu, *tour2d_io_menu;

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

      case TOUR2D:
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

        io_item = submenu_make ("_I/O", 'I', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (io_item), rotation_io_menu); 
        if (firsttime_io) {
          submenu_insert (io_item, menubar, -1);
          firsttime_io = false;
        }
        break;

      case TOUR2D:
        tour2d_menus_make ();

        io_item = submenu_make ("_I/O", 'I', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (io_item), tour2d_io_menu); 
        if (firsttime_io) {
          submenu_insert (io_item, menubar, -1);
          firsttime_io = false;
        }
        break;

      case SCALE :
        scale_menus_make ();

        reset_item = submenu_make ("_Reset", 'R', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (reset_item),
                                   scale_reset_menu); 
        if (firsttime_reset) {
          submenu_insert (reset_item, menubar, -1);
          firsttime_reset = false;
        }
        break;

      case BRUSH :
        brush_menus_make ();

        reset_item = submenu_make ("_Reset", 'R', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (reset_item),
                                   brush_reset_menu); 
        if (firsttime_reset) {
          submenu_insert (reset_item, menubar, -1);
          firsttime_reset = false;
        }

        link_item = submenu_make ("_Link", 'L', main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (link_item),
                                   brush_link_menu); 
        if (firsttime_link) {
          submenu_insert (link_item, menubar, -1);
          firsttime_link = false;
        }

        break;

      case IDENT:
        identify_menus_make ();

        link_item = submenu_make ("_Link", 'L', main_accel_group);
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

gint
mode_get () {
  return mode;
}
gint
projection_get () {
  return projection;
}


/*
 * Set the mode for the current display
*/
void 
mode_set (gint m) {
  displayd *display = current_display;

  mode = m;
  if (mode != prev_mode) {
    /* Add a reference to the widget so it isn't destroyed */
    gtk_widget_ref (control_panel[prev_mode]);
    gtk_container_remove (GTK_CONTAINER (mode_frame),
                          control_panel[prev_mode]);
  
    gtk_frame_set_label (GTK_FRAME (mode_frame), mode_name[mode]);
    gtk_container_add (GTK_CONTAINER (mode_frame), control_panel[mode]);
  }

  /*
   * The projection type is one of P1PLOT, XYPLOT, ROTATE,
   * TOUR2D or COTOUR.  It only changes if another projection
   * type is selected.  (For parcoords and scatmat plots, the
   * value of projection is irrelevant.)
  */
  if (display->displaytype == scatterplot) {
    if (mode <= COTOUR)
      projection = display->cpanel.projection = mode;

    if (projection != prev_projection) {
      scatterplot_show_rulers (display, projection);
      prev_projection = projection;
    }
  }

  prev_mode = mode;

/*  varpanel_refresh ();*/
}

void
mode_activate (splotd *sp, gint m, gboolean state) {
  if (state == off) {

    switch (m) {
      case PCPLOT:
      case P1PLOT:
      case XYPLOT:
      case LINEED:
      case MOVEPTS:
      case COTOUR:
      case ROTATE:
      case TOUR2D:
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
      case TOUR2D:
      case SCALE:
      case IDENT:
        break;

      case BRUSH:
        brush_activate (state);
        break;
    }
  }
}

void
mode_set_cb (gpointer cbd, guint action, GtkWidget *widget)
{
  if (current_display != NULL && current_splot != NULL) {

    sp_event_handlers_toggle (current_splot, off);
    mode_activate (current_splot, mode, off);
    mode_submenus_activate (current_splot, mode, off);

    current_display->cpanel.mode = action;
    mode_set (action);  /* mode = action */

    sp_event_handlers_toggle (current_splot, on);
    mode_activate (current_splot, mode, on);
    mode_submenus_activate (current_splot, mode, on);

    display_tailpipe (current_display);
  }
}

/*-- these will be moved to another file eventually, I'm sure --*/

extern gboolean fileset_read (gchar *);
void
filesel_ok (GtkWidget *w, GtkFileSelection *fs)
{
  gchar *fname = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
  guint action = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (fs),
                 "action"));
g_printerr ("fname=%s, action=%d\n", fname, action);

  switch (action) {
    case 0:  /*-- input: read a new set of files --*/
      if (fileset_read (fname)) {
        displayd *display;

        pipeline_init ();

        /*-- initialize the first display --*/
        display = scatterplot_new (false);
        displays = g_list_append (displays, (gpointer) display);
        display_set_current (display);
        current_splot = (splotd *) g_list_nth_data (current_display->splots, 0);
      }
      break;
    case 1:  /*-- output: extend the current file set --*/
      break;
    case 2:  /*-- output: create a new file set --*/
      break;
  }
}

void
filename_get (gpointer data, guint action, GtkWidget *w) {

  GtkWidget *fs = gtk_file_selection_new ("read ggobi data");
  gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (fs));
  gtk_object_set_data (GTK_OBJECT (fs), "action", GINT_TO_POINTER (action));

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
                      "clicked", GTK_SIGNAL_FUNC (filesel_ok), (gpointer) fs);
                            
  /*-- Ensure that the dialog box is destroyed. --*/
    
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
                             "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             (gpointer) fs);

  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(fs)->cancel_button),
                             "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             (gpointer) fs);
    
  gtk_widget_show (fs);
}

static GtkItemFactoryEntry menu_items[] = {
  { "/_File",            NULL,     NULL,          0, "<Branch>" },
  { "/File/Read ...",    NULL,     filename_get,     0 },
  { "/File/Save (extend file set) ...",   
                         NULL,     filename_get,     1 },
  { "/File/Save (new file set) ...",   
                         NULL,     filename_get,     2 },

  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Print",       NULL,     NULL,          0 },
  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Quit",    "<ctrl>Q",     gtk_main_quit, 0 },

  { "/_Window",                            NULL,   
    NULL,        0,    "<Branch>" },
  { "/Window/New scatterplot",                      "",
    display_new, scatterplot },
  { "/Window/New scatterplot matrix",               "",
    display_new, scatmat },
  { "/Window/New parallel coordinates plot",        "",
    display_new, parcoords },
  { "/Window/sep",     NULL,     NULL,          0, "<Separator>" },
  { "/Window/MISSING VALUES PLOTS", NULL, NULL, 0, "" },
  { "/Window/New scatterplot",        "",
    display_new, 3 },
  { "/Window/New scatterplot matrix", "",
    display_new, 4 },

  { "/_Tools",        NULL,         NULL, 0, "<Branch>" },
  { "/Tools/Variable statistics ...", 
                      NULL,         vartable_create,    0, NULL },
  { "/Tools/Variable transformation ...", 
                      NULL,         open_transform_popup, 0, NULL },
  { "/Tools/Variable jittering ...", 
                      NULL,         jitter_popup_open,    0, NULL },
  { "/Tools/sep",     NULL, NULL, 0, "<Separator>" },
  { "/Tools/Hide or exclude ...", 
                      NULL,         NULL, 0, NULL },
  { "/Tools/Sample or subset ...", 
                      NULL,         open_subset_popup,    0, NULL },
  { "/Tools/sep",     NULL, NULL, 0, "<Separator>" },
  { "/Tools/Smooth ...", 
                      NULL,         open_smooth_popup,    0, NULL },

  { "/Tools/Impute missing values ...", 
                      NULL,         NULL, 0, NULL },

  { "/_Display",      NULL,         NULL, 0, "<Branch>" },
  { "/Display/Show tooltips",  
                      "<ctrl>t",         main_display_options_cb, 0, "<CheckItem>" },
  { "/Display/Show control _panel",  
                      "<ctrl>p" ,        main_display_options_cb, 1, "<CheckItem>" },
  { "/Display/sep",   NULL,     NULL,          0, "<Separator>" },

  { "/Display/Brushing",  
                      NULL ,        NULL, 0, "<Branch>" },
  { "/Display/Brushing/Brush jumps to cursor",  
                      NULL ,        brush_options_cb, 0, "<CheckItem>" },
  { "/Display/Brushing/Update linked brushing continuously",  
                      NULL ,        brush_options_cb, 1, "<CheckItem>" },


  {"/_Plots", NULL, NULL, 0, "<Branch>"},
  {
   "/Plots/Displays",    
          NULL, show_display_tree, 0, NULL},


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
                 main_accel_group, window, &menubar, (gpointer) NULL);
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

  /*-- Variable selection panel --*/
  make_varpanel (hbox);

  gtk_widget_show_all (hbox);

  /*-- adjust the size of the main window before it's displayed --*/
  gtk_widget_realize (window);
  gtk_signal_connect (GTK_OBJECT (window),
                      "size_allocate",
                      (GtkSignalFunc) size_allocate_cb,
                      (gpointer) NULL);

  gtk_widget_show_all (window);
}
