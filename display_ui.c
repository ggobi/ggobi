/*-- display_ui.c --*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static GtkWidget *display_menu_item;


static void
display_open_cb (GtkWidget *w, datad *d)
{
  ggobid *gg = GGobiFromWidget (w, true);
  gint display_type = GPOINTER_TO_INT
    (gtk_object_get_data (GTK_OBJECT (w), "displaytype"));
  gboolean missing_p = GPOINTER_TO_INT
    (gtk_object_get_data (GTK_OBJECT (w), "missing_p"));

g_printerr ("nds = %d, nrows=%d, type=%d, missing=%d\n",
g_slist_length (gg->d), d->nrows, display_type, missing_p);

  /*-- Should the menu be destroyed here?  Doesn't seem to hurt. --*/
  gtk_widget_destroy (w->parent);

  display_create (display_type, missing_p, d, gg);
}

GtkWidget *
display_menu_build (ggobid *gg)
{
  GtkWidget *menu = NULL;
  GtkWidget *item;
  gint nd = g_slist_length (gg->d);
  datad *d0 = (datad *) gg->d->data;
  gint k;
  GtkWidget *submenu, *anchor;
  gchar *lbl;

  if (nd > 0) {
    menu = gtk_menu_new ();
    gtk_object_set_data (GTK_OBJECT (menu), "top", display_menu_item);

    if (nd == 1) {
      item = CreateMenuItem (menu, "New scatterplot",
        NULL, NULL, gg->main_menubar, gg->main_accel_group,
        GTK_SIGNAL_FUNC (display_open_cb), (gpointer) d0, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "displaytype", GINT_TO_POINTER (scatterplot));
      gtk_object_set_data (GTK_OBJECT (item),
        "missing_p", GINT_TO_POINTER (0));

    } else {  /*-- prepare the menu for multiple data matrices --*/

      submenu = gtk_menu_new ();
      anchor = CreateMenuItem (menu, "New scatterplot",
        NULL, NULL, gg->main_menubar, NULL, NULL, NULL, NULL);

      for (k=0; k<nd; k++) { 
        lbl = g_strdup_printf ("data matrix %d", k);
        item = CreateMenuItem (submenu, lbl,
          NULL, NULL, menu, gg->main_accel_group,
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
      item = CreateMenuItem (menu, "New scatterplot matrix",
        NULL, NULL, gg->main_menubar, gg->main_accel_group,
        GTK_SIGNAL_FUNC (display_open_cb), (gpointer) d0, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "displaytype", GINT_TO_POINTER (scatmat));
      gtk_object_set_data (GTK_OBJECT (item),
        "missing_p", GINT_TO_POINTER (0));

    } else {  /*-- prepare the menu for multiple data matrices --*/

      submenu = gtk_menu_new ();
      anchor = CreateMenuItem (menu, "New scatterplot matrix",
        NULL, NULL, gg->main_menubar, NULL, NULL, NULL, NULL);

      for (k=0; k<nd; k++) { 
        lbl = g_strdup_printf ("data matrix %d", k);
        item = CreateMenuItem (submenu, lbl,
          NULL, NULL, menu, gg->main_accel_group,
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
      item = CreateMenuItem (menu, "New parallel coordinates plot",
        NULL, NULL, gg->main_menubar, gg->main_accel_group,
        GTK_SIGNAL_FUNC (display_open_cb), (gpointer) d0, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "displaytype", GINT_TO_POINTER (parcoords));
      gtk_object_set_data (GTK_OBJECT (item),
        "missing_p", GINT_TO_POINTER (0));

    } else {  /*-- prepare the menu for multiple data matrices --*/

      submenu = gtk_menu_new ();
      anchor = CreateMenuItem (menu, "New parallel coordinates plot",
        NULL, NULL, gg->main_menubar, NULL, NULL, NULL, NULL);

      for (k=0; k<nd; k++) { 
        lbl = g_strdup_printf ("data matrix %d", k);
        item = CreateMenuItem (submenu, lbl,
          NULL, NULL, menu, gg->main_accel_group,
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
    CreateMenuItem (menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

    /*-- add a title --*/
    CreateMenuItem (menu, "MISSING VALUES PLOTS",
      "", "", NULL, NULL, NULL, NULL, gg);

    if (nd == 1) {
      item = CreateMenuItem (menu, "New scatterplot",
        NULL, NULL, gg->main_menubar, gg->main_accel_group,
        GTK_SIGNAL_FUNC (display_open_cb), (gpointer) d0, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "displaytype", GINT_TO_POINTER (scatterplot));
      gtk_object_set_data (GTK_OBJECT (item),
        "missing_p", GINT_TO_POINTER (1));
    } 

    if (nd == 1) {
      item = CreateMenuItem (menu, "New scatterplot matrix",
        NULL, NULL, gg->main_menubar, gg->main_accel_group,
        GTK_SIGNAL_FUNC (display_open_cb), (gpointer) d0, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "displaytype", GINT_TO_POINTER (scatmat));
      gtk_object_set_data (GTK_OBJECT (item),
        "missing_p", GINT_TO_POINTER (1));
    } 
  }

  return menu;
}


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

void
display_menu_init (ggobid *gg)
{
  display_menu_item = submenu_make ("_Window", 'W',
    gg->main_accel_group);
  gtk_signal_connect (GTK_OBJECT (display_menu_item),
    "button_press_event",
    GTK_SIGNAL_FUNC (display_menu_open), (gpointer) gg);

  gtk_widget_show (display_menu_item);

  submenu_insert (display_menu_item, gg->main_menubar, 1);
}
