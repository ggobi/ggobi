/* rotate_ui.c */

#include <gtk/gtk.h>
#include <strings.h>

#include "vars.h"

/* external functions */
extern void populate_option_menu (GtkWidget *, gchar **, gint,
                                  GtkSignalFunc);
/*                    */

static GtkWidget *paused_btn, *type_opt, *axis_opt;
static icoords mousepos;

static void rotation_speed_cb (GtkAdjustment *adj, gpointer cbd) {
  g_printerr ("%d\n", ((gint) adj->value));
}
static void scale_set_default_values (GtkScale *scale )
{
  gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_draw_value (scale, false);
}

static void chdir_cb (GtkButton *button)
{
  if (current_display != null) {
    cpaneld *cpanel = &current_display->cpanel;

    cpanel->ro_direction = -1 * cpanel->ro_direction;
  }
}

static void pause_cb (GtkToggleButton *button)
{
  g_printerr ("pause: %d\n", button->active);
}
static void reinit_cb (GtkWidget *w) {
  g_printerr ("reinit\n");
}

static gchar *type_lbl[] = {"Rotate", "Rock", "Interpolate"};
static void type_cb (GtkWidget *w, gpointer cbd)
{
  if (current_display != null) {
    cpaneld *cpanel = &current_display->cpanel;
    gint indx = GPOINTER_TO_INT (cbd);

    cpanel->ro_type = indx;

    g_printerr ("cbd: %s\n", type_lbl[indx]);
  }
}

static gchar *axis_lbl[] = {"Y Axis", "X Axis", "Oblique Axis"};
static void axis_cb (GtkWidget *w, gpointer cbd)
{
  if (current_display != null) {
    cpaneld *cpanel = &current_display->cpanel;
    gint indx = GPOINTER_TO_INT (cbd);

    cpanel->ro_axis = indx;

    g_printerr ("cbd: %s\n", axis_lbl[indx]);
  }
}

/**********************************************************************/
/************************ I/O events **********************************/
/**********************************************************************/

static void rotation_io_cb (GtkWidget *w, gpointer *cbd) {
  gchar *lbl = (gchar *) cbd;
  g_printerr ("cbd: %s\n", lbl);
}

/**********************************************************************/
/********** Handling mouse events in the plot window ******************/
/**********************************************************************/

static gint
motion_notify_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  g_printerr ("(ro_motion_notify_cb)\n");

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  g_printerr ("rotate button_press: %d\n", event->button);

  mousepos.x = event->x;
  mousepos.y = event->y;

  sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                      "motion_notify_event",
                                      (GtkSignalFunc) motion_notify_cb,
                                      (gpointer) sp);
  return true;
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

void
splot_toggle_rotation_handlers (splotd *sp, gpointer stateptr) {
  gboolean state = GPOINTER_TO_INT (stateptr);

  if (state == on) {
    sp->press_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                       "button_press_event",
                                       (GtkSignalFunc) button_press_cb,
                                       (gpointer) sp);
    sp->release_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                         "button_release_event",
                                         (GtkSignalFunc) button_release_cb,
                                         (gpointer) sp);
g_printerr ("adding handlers %d %d\n", sp->press_id, sp->release_id);
  } else {
g_printerr ("disconnecting %d %d\n", sp->press_id, sp->release_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->press_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->release_id);
  }
}

void
display_toggle_rotation_handlers (gboolean state) {

  if (current_display != null)
    g_list_foreach (current_display->splots,
                    (GFunc) splot_toggle_rotation_handlers,
                    GINT_TO_POINTER (state));
}

/**********************************************************************/
/******************* Resetting the main menubar ***********************/
/**********************************************************************/

GtkWidget *rotation_io_menu;

void
make_rotation_menus () {
  GtkWidget *item;

/*
 * I/O menu
*/
  rotation_io_menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ("Save coefficients");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (rotation_io_cb),
                      (gpointer) "write_coeffs");
  gtk_menu_append (GTK_MENU (rotation_io_menu), item);

  item = gtk_menu_item_new_with_label ("Save projection");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (rotation_io_cb),
                      (gpointer) "write_projection");
  gtk_menu_append (GTK_MENU (rotation_io_menu), item);

  item = gtk_menu_item_new_with_label ("Read projection");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (rotation_io_cb),
                      (gpointer) "read_projection");
  gtk_menu_append (GTK_MENU (rotation_io_menu), item);

  gtk_widget_show_all (rotation_io_menu);
}

/**********************************************************************/
/******************* End of main menubar section **********************/
/**********************************************************************/

void
cpanel_rotation_make () {
  GtkWidget *btn, *sbar, *box;
  GtkObject *adj;
  
  control_panel[ROTATE] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (control_panel[ROTATE]), 5);

/*
 * speed scrollbar
*/
  /* value, lower, upper, step_increment, page_increment, page_size */
  /* Note that the page_size value only makes a difference for
   * scrollbar widgets, and the highest value you'll get is actually
   * (upper - page_size). */
  adj = gtk_adjustment_new (1.0, 0.0, 100.0, 1.0, 1.0, 0.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                      GTK_SIGNAL_FUNC (rotation_speed_cb), NULL);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), sbar,
    "Adjust speed of rotation", NULL);
  scale_set_default_values (GTK_SCALE (sbar));

  gtk_box_pack_start (GTK_BOX (control_panel[ROTATE]), sbar,
    false, false, 1);

/*
 * Box to hold 'pause' toggle and 'reinit' button
*/
  box = gtk_hbox_new (true, 2);

  paused_btn = gtk_check_button_new_with_label ("Pause");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), paused_btn,
    "Stop rotation temporarily", NULL);
  gtk_signal_connect (GTK_OBJECT (paused_btn), "toggled",
                     GTK_SIGNAL_FUNC (pause_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (box), paused_btn, true, true, 1);

  btn = gtk_button_new_with_label ("Reinit");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
    "Reset projection", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                     GTK_SIGNAL_FUNC (reinit_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  gtk_box_pack_start (GTK_BOX (control_panel[ROTATE]), box, false, false, 1);

/*
 * Button to change direction
*/
  btn = gtk_button_new_with_label ("Change direction");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
    "Change direction of rotation", NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[ROTATE]), btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (chdir_cb), NULL);

/*
 * option menu: rotate/rock/interpolate
*/
  type_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), type_opt,
    "Rotate freely, rock locally, or interpolate between two orthogonal projections",
    NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[ROTATE]),
                      type_opt, false, false, 0);
  populate_option_menu (type_opt, type_lbl,
                        sizeof (type_lbl) / sizeof (gchar *),
                        type_cb);

/*
 * option menu: y/x/oblique axis
*/
  axis_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), axis_opt,
    "Choose axis of rotation",   NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[ROTATE]),
                      axis_opt, false, false, 0);
  populate_option_menu (axis_opt, axis_lbl,
                        sizeof (axis_lbl) / sizeof (gchar *),
                        axis_cb);
  gtk_widget_show_all (control_panel[ROTATE]);
}

/**********************************************************************/
/********************** Control panel section *************************/
/**********************************************************************/

void
init_rotation_cpanel (cpaneld *cpanel) {
 cpanel->ro_paused_p = false;
 cpanel->ro_axis = RO_OBLIQUE;
 cpanel->ro_type = RO_ROTATE;
 cpanel->ro_direction = FORWARD;
}

void
show_rotation_cpanel (cpaneld *cpanel) {

  gtk_option_menu_set_history (GTK_OPTION_MENU (type_opt),
                               cpanel->ro_type);
  gtk_option_menu_set_history (GTK_OPTION_MENU (axis_opt),
                               cpanel->ro_axis);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (paused_btn),
                                cpanel->ro_paused_p);
}

