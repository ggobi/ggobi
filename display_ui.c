/*-- display_ui.c --*/
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
#include "vars.h"
#include "externs.h"

void
display_set_position (displayd *display, ggobid *gg)
{
  gint x, y, width, height;
  gint posx, posy;

  /*-- get the size and position of the gg->main_window) --*/
  gdk_window_get_root_origin (gg->main_window->window, &x, &y);
  gdk_window_get_size (gg->main_window->window, &width, &height);

  gtk_widget_realize (display->window);
  if (x==0 && y==0) {  /*-- can't get any info for the first display --*/
    posx = gdk_screen_width()/4;
    posy = gdk_screen_height()/4;
  } else {
    posx = x+(3*width)/4;
    posy = y+(3*height)/4;
  }

  gtk_widget_set_uposition (display->window, posx, posy);
}

static void
display_open_cb (GtkWidget *w, datad *d)
{
  ggobid *gg = GGobiFromWidget (w, true);
  gint display_type = GPOINTER_TO_INT
    (gtk_object_get_data (GTK_OBJECT (w), "displaytype"));
  gboolean missing_p = GPOINTER_TO_INT
    (gtk_object_get_data (GTK_OBJECT (w), "missing_p"));

  display_create (display_type, missing_p, d, gg);
}


void
display_menu_build (ggobid *gg)
{
  GtkWidget *item;
  gint nd = g_slist_length (gg->d);
  datad *d0;
  gint k;
  GtkWidget *submenu, *anchor;
  gchar *lbl;

  if(gg == NULL || gg->d == NULL)
      return;

  d0 = (datad *) gg->d->data;
  if (gg->display_menu != NULL)
    gtk_widget_destroy (gg->display_menu);

  if (nd > 0) {
    gg->display_menu = gtk_menu_new ();

    if (nd == 1) {
      item = CreateMenuItem (gg->display_menu, "New scatterplot",
        NULL, NULL, gg->main_menubar, gg->main_accel_group,
        GTK_SIGNAL_FUNC (display_open_cb), (gpointer) d0, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "displaytype", GINT_TO_POINTER (scatterplot));
      gtk_object_set_data (GTK_OBJECT (item),
        "missing_p", GINT_TO_POINTER (0));

    } else {  /*-- prepare the cascading menu for multiple data matrices --*/

      submenu = gtk_menu_new ();
      anchor = CreateMenuItem (gg->display_menu, "New scatterplot",
        NULL, NULL, gg->main_menubar, NULL, NULL, NULL, NULL);

      for (k=0; k<nd; k++) { 
        datad *d = (datad*) g_slist_nth_data (gg->d, k);

        /*-- add an item for each datad with variables --*/
        if (g_slist_length (d->vartable) > 0) {
          lbl = datasetName (d, gg);
          item = CreateMenuItem (submenu, lbl,
            NULL, NULL, gg->display_menu, gg->main_accel_group,
            GTK_SIGNAL_FUNC (display_open_cb),
            d, gg);
          gtk_object_set_data (GTK_OBJECT (item),
            "displaytype", GINT_TO_POINTER (scatterplot));
          gtk_object_set_data (GTK_OBJECT (item),
            "missing_p", GINT_TO_POINTER (0));
          g_free (lbl);
        }
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
        datad *d = (datad*) g_slist_nth_data (gg->d, k);

        /*-- add an item for each datad with variables --*/
        if (g_slist_length (d->vartable) > 0) {
          lbl = datasetName (d, gg);
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
      anchor = CreateMenuItem (gg->display_menu,
        "New parallel coordinates plot",
        NULL, NULL, gg->main_menubar, NULL, NULL, NULL, NULL);

      for (k=0; k<nd; k++) { 
        datad *d = (datad*) g_slist_nth_data (gg->d, k);

        /*-- add an item for each datad with variables --*/
        if (g_slist_length (d->vartable) > 0) {
          lbl = datasetName (d, gg);
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
      }

      gtk_menu_item_set_submenu (GTK_MENU_ITEM (anchor), submenu);
    } 

    if (nd == 1) {
      item = CreateMenuItem (gg->display_menu, "New time series plot",
        NULL, NULL, gg->main_menubar, gg->main_accel_group,
        GTK_SIGNAL_FUNC (display_open_cb), (gpointer) d0, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "displaytype", GINT_TO_POINTER (tsplot));
      gtk_object_set_data (GTK_OBJECT (item),
        "missing_p", GINT_TO_POINTER (0));

    } else {  /*-- prepare the menu for multiple data matrices --*/

      submenu = gtk_menu_new ();
      anchor = CreateMenuItem (gg->display_menu,
        "New time series plot",
        NULL, NULL, gg->main_menubar, NULL, NULL, NULL, NULL);

      for (k=0; k<nd; k++) { 
        datad *d = (datad*) g_slist_nth_data (gg->d, k);

        /*-- add an item for each datad with variables --*/
        if (g_slist_length (d->vartable) > 0) {
          lbl = datasetName (d, gg);
          item = CreateMenuItem (submenu, lbl,
            NULL, NULL, gg->display_menu, gg->main_accel_group,
            GTK_SIGNAL_FUNC (display_open_cb),
            g_slist_nth_data (gg->d, k), gg);
          gtk_object_set_data (GTK_OBJECT (item),
            "displaytype", GINT_TO_POINTER (tsplot));
          gtk_object_set_data (GTK_OBJECT (item),
            "missing_p", GINT_TO_POINTER (0));
          g_free (lbl);
        }
      }

      gtk_menu_item_set_submenu (GTK_MENU_ITEM (anchor), submenu);
    } 

    /*-- add a separator --*/
    CreateMenuItem (gg->display_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

    /*-- add a title --*/
    CreateMenuItem (gg->display_menu, "MISSING VALUES DISPLAYS",
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

void
display_menu_init (ggobid *gg)
{
  gg->display_menu_item = submenu_make ("_Display", 'D',
    gg->main_accel_group);

  gtk_widget_show (gg->display_menu_item);

  submenu_insert (gg->display_menu_item, gg->main_menubar, 1);
}



