/* display.c */

#include <gtk/gtk.h>
#include <strings.h>

#include "vars.h"

/* external functions */
extern void show_control_panel (cpaneld *);
extern displayd * scatterplot_new ();
extern displayd * scatmat_new ();
extern displayd * parcoords_new ();
extern void set_mode (gint);
extern void activate_mode (gint, gboolean);
extern void make_scatterplot_menus (GtkAccelGroup *, GtkSignalFunc);
extern void make_scatmat_menus (GtkAccelGroup *, GtkSignalFunc);
extern void make_parcoords_menus (GtkAccelGroup *, GtkSignalFunc);
extern void set_mode_cb (gpointer, guint, GtkWidget  *);
extern GtkWidget * make_submenu (gchar *, gint, GtkAccelGroup *);
extern void insert_submenu (GtkWidget *, GtkWidget *, gint);
extern void destroy_submenu (GtkWidget  *);
extern void splot_world_to_plane (cpaneld *, splotd *);
extern void splot_plane_to_screen (cpaneld *, splotd *);
extern void splot_free (splotd *);
extern void quick_message (gchar *);

void display_set_current (displayd *);
/* */


void
display_new (gpointer   cbd,
             guint      action,
             GtkWidget  *widget)
{
  displayd *display;

  /*
   * Remove all signal handlers from the current splot
  */

  /* removes signal handlers AND menus */
g_printerr ("current_display: type %d mode %d\n",
current_display->displaytype, current_display->cpanel.mode);
  /*activate_mode (current_display->cpanel.mode, off); */

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

  splot_set_current ((splotd *) g_list_nth_data (current_display->splots, 0));

  /*
   * scatterplot starts in a mode without signal handlers, but
   * I may need to activate later, if pcplot or matplot starts with
   * some kind of handler turned on
  */
  set_mode (current_display->cpanel.mode);  /* don't activate */
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
    if (current_display == display)
      display_set_current (g_list_nth_data (displays, 0));

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

void
display_reproject (displayd *display) {
  GList *splist = display->splots;
  splotd *sp;
  gboolean rval = false;
  cpaneld *cpanel;

  while (splist) {
    sp = (splotd *) splist->data;
    cpanel = &display->cpanel;

    splot_world_to_plane (cpanel, sp);
    splot_plane_to_screen (cpanel, sp);
    gtk_signal_emit_by_name (GTK_OBJECT (sp->da), "expose_event",
                            (gpointer) sp, (gpointer) &rval);

    splist = splist->next;
  }
}
