/*-- display_ui.c --*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static void
display_open_cb (GtkWidget *w, datad *d)
{
  ggobid *gg = GGobiFromWidget (w, true);
  gint display_type = GPOINTER_TO_INT
    (gtk_object_get_data (GTK_OBJECT (w), "displaytype"));
  gboolean missing_p = GPOINTER_TO_INT
    (gtk_object_get_data (GTK_OBJECT (w), "missing_p"));

  /*-- Should the menu be destroyed here?  Doesn't seem to hurt. --*/
  gtk_widget_destroy (w->parent);

  display_create (display_type, missing_p, d, gg);
}

void
display_menu_build (ggobid *gg)
{
  GtkWidget *item;
  gint nd = g_slist_length (gg->d);
  datad *d0 = (datad *) gg->d->data;
  gint k;
  GtkWidget *submenu, *anchor;
  gchar *lbl;

  if (gg->display_menu != NULL)
    gtk_widget_destroy (gg->display_menu);

  if (nd > 0) {
    gg->display_menu = gtk_menu_new ();

/*-- used in positioning popup menus; no longer needed here --*/
/*
    gtk_object_set_data (GTK_OBJECT (gg->display_menu),
      "top", gg->display_menu_item);
*/

    if (nd == 1) {
      item = CreateMenuItem (gg->display_menu, "New scatterplot",
        NULL, NULL, gg->main_menubar, gg->main_accel_group,
        GTK_SIGNAL_FUNC (display_open_cb), (gpointer) d0, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "displaytype", GINT_TO_POINTER (scatterplot));
      gtk_object_set_data (GTK_OBJECT (item),
        "missing_p", GINT_TO_POINTER (0));

    } else {  /*-- prepare the menu for multiple data matrices --*/

      submenu = gtk_menu_new ();
      anchor = CreateMenuItem (gg->display_menu, "New scatterplot",
        NULL, NULL, gg->main_menubar, NULL, NULL, NULL, NULL);

      for (k=0; k<nd; k++) { 
        lbl = g_strdup_printf ("data matrix %d", k);
        item = CreateMenuItem (submenu, lbl,
          NULL, NULL, gg->display_menu, gg->main_accel_group,
          GTK_SIGNAL_FUNC (display_open_cb),
          g_slist_nth_data (gg->d, k), gg);
        gtk_object_set_data (GTK_OBJECT (item),
          "displaytype", GINT_TO_POINTER (scatterplot));
        gtk_object_set_data (GTK_OBJECT (item),
          "missing_p", GINT_TO_POINTER (0));
        g_free (lbl);
      }

      gtk_menu_item_set_submenu (GTK_MENU_ITEM (anchor), submenu);
    }

    if (nd == 1) {
      item = CreateMenuItem (gg->display_menu, "New scatterplot matrix",
        NULL, NULL, gg->main_menubar, gg->main_accel_group,
        GTK_SIGNAL_FUNC (display_open_cb), (gpointer) d0, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "displaytype", GINT_TO_POINTER (scatmat));
      gtk_object_set_data (GTK_OBJECT (item),
        "missing_p", GINT_TO_POINTER (0));

    } else {  /*-- prepare the menu for multiple data matrices --*/

      submenu = gtk_menu_new ();
      anchor = CreateMenuItem (gg->display_menu, "New scatterplot matrix",
        NULL, NULL, gg->main_menubar, NULL, NULL, NULL, NULL);

      for (k=0; k<nd; k++) { 
        lbl = g_strdup_printf ("data matrix %d", k);
        item = CreateMenuItem (submenu, lbl,
          NULL, NULL, gg->display_menu, gg->main_accel_group,
          GTK_SIGNAL_FUNC (display_open_cb),
          g_slist_nth_data (gg->d, k), gg);
        gtk_object_set_data (GTK_OBJECT (item),
          "displaytype", GINT_TO_POINTER (scatmat));
        gtk_object_set_data (GTK_OBJECT (item),
          "missing_p", GINT_TO_POINTER (0));
        g_free (lbl);
      }

      gtk_menu_item_set_submenu (GTK_MENU_ITEM (anchor), submenu);
    } 

    if (nd == 1) {
      item = CreateMenuItem (gg->display_menu, "New parallel coordinates plot",
        NULL, NULL, gg->main_menubar, gg->main_accel_group,
        GTK_SIGNAL_FUNC (display_open_cb), (gpointer) d0, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "displaytype", GINT_TO_POINTER (parcoords));
      gtk_object_set_data (GTK_OBJECT (item),
        "missing_p", GINT_TO_POINTER (0));

    } else {  /*-- prepare the menu for multiple data matrices --*/

      submenu = gtk_menu_new ();
      anchor = CreateMenuItem (gg->display_menu, "New parallel coordinates plot",
        NULL, NULL, gg->main_menubar, NULL, NULL, NULL, NULL);

      for (k=0; k<nd; k++) { 
        lbl = g_strdup_printf ("data matrix %d", k);
        item = CreateMenuItem (submenu, lbl,
          NULL, NULL, gg->display_menu, gg->main_accel_group,
          GTK_SIGNAL_FUNC (display_open_cb),
          g_slist_nth_data (gg->d, k), gg);
        gtk_object_set_data (GTK_OBJECT (item),
          "displaytype", GINT_TO_POINTER (parcoords));
        gtk_object_set_data (GTK_OBJECT (item),
          "missing_p", GINT_TO_POINTER (0));
        g_free (lbl);
      }

      gtk_menu_item_set_submenu (GTK_MENU_ITEM (anchor), submenu);
    } 

    /*-- add a separator --*/
    CreateMenuItem (gg->display_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

    /*-- add a title --*/
    CreateMenuItem (gg->display_menu, "MISSING VALUES PLOTS",
      "", "", NULL, NULL, NULL, NULL, gg);

    if (nd == 1) {
      item = CreateMenuItem (gg->display_menu, "New scatterplot",
        NULL, NULL, gg->main_menubar, gg->main_accel_group,
        GTK_SIGNAL_FUNC (display_open_cb), (gpointer) d0, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "displaytype", GINT_TO_POINTER (scatterplot));
      gtk_object_set_data (GTK_OBJECT (item),
        "missing_p", GINT_TO_POINTER (1));
    } 

    if (nd == 1) {
      item = CreateMenuItem (gg->display_menu, "New scatterplot matrix",
        NULL, NULL, gg->main_menubar, gg->main_accel_group,
        GTK_SIGNAL_FUNC (display_open_cb), (gpointer) d0, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "displaytype", GINT_TO_POINTER (scatmat));
      gtk_object_set_data (GTK_OBJECT (item),
        "missing_p", GINT_TO_POINTER (1));
    } 
  }

  /*-- these two lines replace gtk_menu_popup --*/
  gtk_widget_show_all (gg->display_menu);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->display_menu_item),
                             gg->display_menu);
}

/*
 * This was part of the popup menu solution, but that is unsatisfactory
 * because then the menu isn't visible when the mouse floats over the
 * menu bar.  Instead I need to destroy it and rebuild it whenever the
 * length or composition of gg->d changes.  At the moment, it's just
 * being built once.
*/
/*
static gint
display_menu_open (GtkWidget *w, GdkEvent *event, ggobid *gg)
{
  GtkWidget *display_menu;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    if (bevent->button == 1) {
      display_menu = display_menu_build (gg);
      gtk_menu_popup (GTK_MENU (display_menu), NULL, NULL,
        position_popup_menu, gg,
        bevent->button, bevent->time);

      return true;
    }
  }
  return false;
}
*/

void
display_menu_init (ggobid *gg)
{
  gg->display_menu_item = submenu_make ("_Window", 'W',
    gg->main_accel_group);

/*-- part of the popup menu strategy; now abandoned --*/
/*
  gtk_signal_connect (GTK_OBJECT (gg->display_menu_item),
    "button_press_event",
    GTK_SIGNAL_FUNC (display_menu_open), (gpointer) gg);
*/

  gtk_widget_show (gg->display_menu_item);

  submenu_insert (gg->display_menu_item, gg->main_menubar, 1);
}
