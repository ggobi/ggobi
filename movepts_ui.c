/* movepts_ui.c */

#include <gtk/gtk.h>
#include <strings.h>

#include "vars.h"
#include "externs.h"

static void reset_all_cb (GtkButton *button)
{
  g_printerr ("reset all\n");
}
static void undo_last_cb (GtkButton *button)
{
  g_printerr ("undo last\n");
}

static void use_groups_cb (GtkToggleButton *button)
{
  g_printerr ("use group: %d\n", button->active);
}

static gchar *mdir_lbl[] = {"Both", "Vertical", "Horizontal"};
static void mdir_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", mdir_lbl[indx]);
}

/*--------------------------------------------------------------------*/
/*          Handling and mouse events in the plot window              */
/*--------------------------------------------------------------------*/

static gint
motion_notify_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  current_splot = sp;

  g_printerr ("(mp_motion_notify_cb) sp size %d %d\n", sp->max.x, sp->max.y);

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  current_display = (displayd *) sp->displayptr;
  current_splot = sp;

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

  current_splot = sp;

  mousepos.x = event->x;
  mousepos.y = event->y;

  gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->motion_id);

  return retval;
}

void
movepts_event_handlers_toggle (splotd *sp, gboolean state) {
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
cpanel_movepts_make () {
  GtkWidget *btn, *opt, *box, *hb, *lbl;
  
  control_panel[MOVEPTS] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (control_panel[MOVEPTS]), 5);

/*
 * option menu: direction of motion 
*/
  hb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (control_panel[MOVEPTS]), hb, false, false, 0);

  lbl = gtk_label_new ("Direction of motion:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 1);
  gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
    "Move freely, or constrain the motion vertically or horizontally",
    NULL);
  populate_option_menu (opt, mdir_lbl,
                        sizeof (mdir_lbl) / sizeof (gchar *),
                        mdir_cb);
  gtk_box_pack_start (GTK_BOX (hb), opt, false, false, 0);

/*
 * Use group toggle
*/
  btn = gtk_check_button_new_with_label ("Use 'group' var");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
    "Use variable groups: move an entire group together", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "toggled",
                     GTK_SIGNAL_FUNC (use_groups_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[MOVEPTS]), btn, false, false, 1);

/*
 * Box to hold reset buttons
*/
  box = gtk_hbox_new (true, 2);

  btn = gtk_button_new_with_label ("Reset all");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
    "Reset all points to their original positions", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                     GTK_SIGNAL_FUNC (reset_all_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (box), btn, false, false, 1);

  btn = gtk_button_new_with_label ("Undo last");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
    "Undo the previous move", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                     GTK_SIGNAL_FUNC (undo_last_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (box), btn, false, false, 1);

  gtk_box_pack_start (GTK_BOX (control_panel[MOVEPTS]), box, false, false, 1);

  gtk_widget_show_all (control_panel[MOVEPTS]);
}
