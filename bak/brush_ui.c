
#include <gtk/gtk.h>
#include <strings.h>

#include "vars.h"

/* external functions */
extern void make_symbol_window ();
extern void populate_option_menu (GtkWidget *, gchar **, gint,
                                  GtkSignalFunc);
/*                    */

static icoords mousepos;

static void open_symbol_window_cb (GtkWidget *w) {
  make_symbol_window ();
}

static gchar *mode_lbl[] = {"Persistent", "Transient", "Undo"};
static void brush_mode_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", mode_lbl[indx]);
}

static gchar *scope_lbl[] = {"Points", "Lines", "Points and lines"};
static void brush_scope_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", scope_lbl[indx]);
}

static void
brush_reset_cb (GtkWidget *w, gpointer cbd)
{
  gchar *lbl = (gchar *) cbd;
  g_printerr ("cbd: %s\n", lbl);
}
static void
brush_link_cb (GtkCheckMenuItem *w, gpointer cbd)
{
  gchar *lbl = (gchar *) cbd;
  g_printerr ("state: %d, cbd: %s\n", w->active, lbl);
}
void
brush_options_cb (gpointer data, guint action, GtkCheckMenuItem *w)
{
/*
 * action 0 : Brush jumps to cursor
 * action 1 : Update linked brushing continuously
*/
  g_printerr ("action: %d, state: %d\n", action,  w->active);
}

/******************** Mouse events *************************/

static gint
motion_notify_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  g_printerr ("(br_motion_notify_cb) %f %f\n", event->x, event->y);

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;
  current_splot = sp;
  current_display = (displayd *) sp->displayptr;

  mousepos.x = event->x;
  mousepos.y = event->y;
g_printerr ("brush button press\n");

  sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                     "motion_notify_event",
                                     (GtkSignalFunc) motion_notify_cb,
                                     (gpointer) sp);
  return retval;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;

  mousepos.x = event->x;
  mousepos.y = event->y;

  gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->motion_id);

  return retval;
}

/**********************************************************************/
/******************* Resetting the main menubar ***********************/
/**********************************************************************/

void
splot_toggle_brush_handlers (splotd *sp, gboolean state) {

  if (state == on) {
    sp->press_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                      "button_press_event",
                                      (GtkSignalFunc) button_press_cb,
                                      (gpointer) sp);
    sp->release_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                        "button_release_event",
                                        (GtkSignalFunc) button_release_cb,
                                        (gpointer) sp);
  } else {
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->press_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->release_id);
  }
}

void
display_toggle_brush_handlers (gboolean state) {
  GList *splist = current_display->splots;
  splotd *sp;

  while (splist) {
    sp = (splotd *) splist->data;
    splot_toggle_brush_handlers (sp, state);
    splist = splist->next;
  }
}

GtkWidget *brush_reset_menu, *brush_link_menu;
void
make_brush_menus () {
  GtkWidget *item;

/*
 * Reset menu
*/
  brush_reset_menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ("Reset point colors");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) "pointcolors");
  gtk_menu_append (GTK_MENU (brush_reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset brush size");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) "brushsize");
  gtk_menu_append (GTK_MENU (brush_reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset linecolors");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) "linecolors");
  gtk_menu_append (GTK_MENU (brush_reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset glyphs");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) "glyphs");
  gtk_menu_append (GTK_MENU (brush_reset_menu), item);

  gtk_widget_show_all (brush_reset_menu);

/*
 * Link menu
*/
  brush_link_menu = gtk_menu_new ();

  item = gtk_check_menu_item_new_with_label ("Link points <-> points");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "p2p");
  gtk_menu_append (GTK_MENU (brush_link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);

  item = gtk_check_menu_item_new_with_label ("Link lines <-> lines");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "l2l");
  gtk_menu_append (GTK_MENU (brush_link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);

  item = gtk_check_menu_item_new_with_label ("Link points <-> lines");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "p2l");
  gtk_menu_append (GTK_MENU (brush_link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);

  item = gtk_check_menu_item_new_with_label ("Link color brushing");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "color");
  gtk_menu_append (GTK_MENU (brush_link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);
  item = gtk_check_menu_item_new_with_label ("Link glyph brushing");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "glyph");
  gtk_menu_append (GTK_MENU (brush_link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);

  gtk_widget_show_all (brush_link_menu);
}

void
cpanel_brush_make () {
  GtkWidget *mode_opt;
  GtkWidget *scope_opt;
  GtkWidget *btn;
  
  control_panel[BRUSH] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (control_panel[BRUSH]), 5);

/*
 * make an option menu for setting the brushing mode
*/
  scope_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), scope_opt,
    "Brush points only, lines only, or both points and lines", NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[BRUSH]),
                      scope_opt, false, false, 0);
  populate_option_menu (scope_opt, scope_lbl,
                        sizeof (scope_lbl) / sizeof (gchar *),
                        brush_scope_cb);
  /* points only */
  gtk_option_menu_set_history (GTK_OPTION_MENU (scope_opt), 0); 
  

/*
 * option menu for setting the brushing persistence
*/
  mode_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), mode_opt,
    "Persistent, transient or undo brushing", NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[BRUSH]),
                      mode_opt, false, false, 0);
  populate_option_menu (mode_opt, mode_lbl,
                        sizeof (mode_lbl) / sizeof (gchar *),
                        brush_mode_cb);
  gtk_option_menu_set_history (GTK_OPTION_MENU (mode_opt), 1);  /* transient */

/*
 * button for opening symbol panel
*/
  btn = gtk_button_new_with_label ("Choose symbol ...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips),
                        btn, "Open panel for choosing color and glyph", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (open_symbol_window_cb),
                      (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[BRUSH]),
                      btn, false, false, 1);

/*
 * button for opening hide/exclude panel
*/
  btn = gtk_button_new_with_label ("Hide or exclude ...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips),
                        btn,
                        "Open panel for hiding or excluding brushed groups",
                        NULL);
/*
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (open_symbol_window_cb), (gpointer) NULL);
*/
  gtk_box_pack_start (GTK_BOX (control_panel[BRUSH]),
                      btn, false, false, 1);

  gtk_widget_show_all (control_panel[BRUSH]);
}

