/* lineedit_ui.c */

#include <gtk/gtk.h>
#include <strings.h>

#include "vars.h"

static icoords mousepos;

/**********************************************************************/
/********** Respond to buttons and menus in the panel *****************/
/**********************************************************************/

static void addordelete_cb (GtkToggleButton *button)
{
  g_printerr("active %d\n", button->active);
}
static void show_lines_cb (GtkToggleButton *button)
{
  g_printerr("active %d\n", button->active);
}
static void remove_lines_cb (GtkToggleButton *button)
{
  g_printerr("move all lines\n");
}
static void include_missings_cb (GtkToggleButton *button)
{
  g_printerr("active %d\n", button->active);
}

/**********************************************************************/
/********** Handling and mouse events in the plot window **************/
/**********************************************************************/

static gint
motion_notify_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  g_printerr ("(le_motion_notify_cb)\n");

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  current_splot = sp;
  current_display = (displayd *) sp->displayptr;

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
splot_toggle_lineedit_handlers (splotd *sp, gpointer stateptr) {
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
  } else {
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->press_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->release_id);
  }
}

void
display_toggle_lineedit_handlers (gboolean state) {
  if (current_display != null && current_display->splots != null)
    g_list_foreach (current_display->splots,
                    (GFunc) splot_toggle_lineedit_handlers,
                    GINT_TO_POINTER (state));
}

void
cpanel_lineedit_make () {
  GtkWidget *btn;
  GtkWidget *hb, *radio1, *radio2;
  GSList *group;
  
  control_panel[LINEED] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (control_panel[LINEED]), 5);

/*
 * Radio group in a box: add/delete buttons
*/
  hb = gtk_hbox_new (true, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hb), 3);
  gtk_box_pack_start (GTK_BOX (control_panel[LINEED]), hb, false, false, 0);

  radio1 = gtk_radio_button_new_with_label (NULL, "Add");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio1), TRUE);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), radio1,
    "Add new line segments using the mouse", NULL);
  gtk_signal_connect (GTK_OBJECT (radio1), "toggled",
                      GTK_SIGNAL_FUNC (addordelete_cb), NULL);
  gtk_box_pack_start (GTK_BOX (hb), radio1, false, false, 0);

  group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio1));

  radio2 = gtk_radio_button_new_with_label (group, "Delete");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), radio2,
    "Delete line segments using the mouse", NULL);
  gtk_box_pack_start (GTK_BOX (hb), radio2, false, false, 0);

/*
 * Show lines toggle
*/
  btn = gtk_check_button_new_with_label ("Show lines");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
    "Show connected lines", NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[LINEED]),
                      btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "toggled",
                      GTK_SIGNAL_FUNC (show_lines_cb), NULL);

/*
 * Remove lines button
*/
  btn = gtk_button_new_with_label ("Remove all lines");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
    "Remove all lines", NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[LINEED]),
                      btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (remove_lines_cb), NULL);
/*
 * Including missings togle
*/
  btn = gtk_check_button_new_with_label ("Include missings");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
    "Include missing values", NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[LINEED]),
                      btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "toggled",
                      GTK_SIGNAL_FUNC (include_missings_cb), NULL);

  gtk_widget_show_all (control_panel[LINEED]);
}

