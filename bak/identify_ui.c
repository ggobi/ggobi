
#include <gtk/gtk.h>
#include <strings.h>

#include "vars.h"

GtkWidget *identify_link_menu;
icoords cursor_pos;

static void id_remove_labels_cb (GtkButton *button)
{
  g_printerr("removing labels\n");
}
static void id_all_sticky_cb ( GtkScale *scale )
{
  g_printerr("make all labels sticky\n");
}
static void identify_link_cb (GtkCheckMenuItem *w, gpointer cbd)
{
  gchar *lbl = (gchar *) cbd;
  g_printerr("state: %d, cbd: %s\n", w->active, lbl);
}

/*************************************************************/
/**************** Handling mouse events **********************/
/*************************************************************/

extern gint find_nearest_point (icoords *, splotd *);
static gint
motion_notify_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gint k;
  static gint prev_nearest;
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);

  gdk_gc_set_foreground (plot_GC, &xg.accent_color);

  cursor_pos.x = event->x;
  cursor_pos.y = event->y;

  if ((k = find_nearest_point (&cursor_pos, sp)) != -1) {
    if (k != prev_nearest) {
      gdk_draw_pixmap (w->window, plot_GC, sp->pixmap,
                       0, 0, 0, 0,
                       w->allocation.width,
                       w->allocation.height);
      gdk_text_extents (style->font,  
/*          "lbl", strlen("lbl"),*/
        xg.rowlab[k], strlen (xg.rowlab[k]),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (w->window, style->font, plot_GC,
/*        sp->screen[k].x+2, sp->screen[k].y-2, "lbl");*/
        sp->screen[k].x+2, sp->screen[k].y-2, xg.rowlab[k]);
    }
    prev_nearest = k;
  }

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  current_splot = sp;
  current_display = (displayd *) sp->displayptr;

  g_printerr("(identify: button_release) %f %f\n", event->x, event->y);

  return true;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  g_printerr("(identify: button_release) %f %f\n", event->x, event->y);

  return true;
}

void
identify_toggle_splot_handlers (splotd *sp, gboolean state) {
  
  if (state == on) {
    sp->press_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                       "button_press_event",
                                       (GtkSignalFunc) button_press_cb,
                                       (gpointer) sp);
    sp->release_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                         "button_release_event",
                                         (GtkSignalFunc) button_release_cb,
                                         (gpointer) sp);
    sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                        "motion_notify_event",
                                        (GtkSignalFunc) motion_notify_cb,
                                        (gpointer) sp);
g_printerr ("connecting %d %d %d\n", sp->press_id, sp->release_id, sp->motion_id);
  } else {
g_printerr ("disconnecting %d %d %d\n", sp->press_id, sp->release_id, sp->motion_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->press_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->release_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->motion_id);
  }

}

/*
 * This is giving me trouble, and I don't really know why.
 * I may have to change it so that only the current splot
 * has event handlers attached to it.
*/
void
display_toggle_identify_handlers (gboolean state) {
  if (current_display != null && current_display->splots != null)
/*    splot_toggle_identify_handlers (current_splot, state);*/
    g_list_foreach (current_display->splots,
                    (GFunc) splot_toggle_identify_handlers,
                    GINT_TO_POINTER (state));
}


/**********************************************************************/
/******************* Resetting the main menubar ***********************/
/**********************************************************************/


void
make_identify_menus() {
  GtkWidget *item;

/*
 * Link menu
*/
  identify_link_menu = gtk_menu_new ();

  item = gtk_check_menu_item_new_with_label("Link identification");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (identify_link_cb),
                      (gpointer) NULL);
  gtk_menu_append (GTK_MENU (identify_link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);
  gtk_widget_show(item);
}

void
cpanel_identify_make() {
  GtkWidget *btn;
  
  control_panel[IDENT] = gtk_vbox_new(false, VBOX_SPACING);
  gtk_container_set_border_width(GTK_CONTAINER(control_panel[IDENT]), 5);

/*
 * button for removing all labels
*/
  btn = gtk_button_new_with_label ("Remove labels");
  gtk_tooltips_set_tip (GTK_TOOLTIPS(xg.tips),
                        btn, "Remove all labels", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (id_remove_labels_cb),
                      (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[IDENT]),
                      btn, false, false, 1);

/*
 * button for making all labels sticky
*/
  btn = gtk_button_new_with_label ("Make all sticky");
  gtk_tooltips_set_tip (GTK_TOOLTIPS(xg.tips),
                        btn, "Make all labels sticky, or persistent", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (id_all_sticky_cb),
                      (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[IDENT]),
                      btn, false, false, 1);

  gtk_widget_show_all (control_panel[IDENT]);
}

