/* lineedit_ui.c */
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
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include "vars.h"
#include "externs.h"


/*--------------------------------------------------------------------*/
/*          Respond to buttons and menus in the panel                 */
/*--------------------------------------------------------------------*/

static void addordelete_cb (GtkToggleButton *button, ggobid *gg)
{
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;

  cpanel->ee_adding_p = button->active;
  cpanel->ee_deleting_p = !button->active;
}
static void undo_last_cb (GtkToggleButton *button)
{
  g_printerr("undo last\n");
}

/*--------------------------------------------------------------------*/
/*          Handling and mouse events in the plot window              */
/*--------------------------------------------------------------------*/

static gint
key_press_cb (GtkWidget *w, GdkEventKey *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot(sp);
  cpaneld *cpanel = &gg->current_display->cpanel;

  /*-- handle the keys for setting the mode and launching generic events --*/
  if (splot_event_handled (w, event, cpanel, sp, gg))
    return true;

  /*-- insert mode-specific key presses (if any) here --*/

  return true;
}

static gint
motion_notify_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp)
{
  displayd *display = sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  datad *d = display->d;
  ggobid *gg = GGobiFromSPlot(sp);
  gboolean button1_p, button2_p;
  gint k;

  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);
  k = find_nearest_point (&sp->mousepos, sp, d, gg);
  d->nearest_point = k;

  if (cpanel->ee_adding_p) {
    if (gg->edgeedit.a == -1) {  /*-- looking for starting point --*/

      if (k != d->nearest_point_prev) {
        displays_plot (NULL, QUICK, gg);
      }

    } else {  /*-- found starting point; looking for ending point --*/

      displays_plot (NULL, QUICK, gg);
      /*-- add a dotted line from gg->edgeedit.a to gg->nearest_point --*/

    }
  }

  d->nearest_point_prev = d->nearest_point;
  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot(sp);
  displayd *display = sp->displayptr;
  datad *d = display->d;
  gboolean button1_p, button2_p;
  mousepos_get_pressed (w, event, &button1_p, &button2_p, sp);

  gg->current_splot = sp;
  gg->current_display = display;
  
  gg->edgeedit.a = d->nearest_point;

  return true;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot (sp);
  displayd *display = sp->displayptr;
  datad *d = display->d;
  datad *e = display->e;

  gg->buttondown = 0;

  sp->mousepos.x = (gint) event->x;
  sp->mousepos.y = (gint) event->y;

  /*
   * add the edge to display->e.  If display->e is NULL, then
   * initialize it first.
   *
   * If record indices are in use, use them; if not, initialize
   * indices for display->d.
  */
  g_printerr ("add the edge from %d to %d\n",
    d->nearest_point, gg->edgeedit.a);

  if (e == NULL)
    g_printerr ("Not yet initializing a new edge set\n");
  else if (d->rowid.id.nels == 0)
    g_printerr ("Not yet initializing new rowids\n");
  else {  
    extern gboolean edge_add (gint, gint, datad *, datad *);
    edge_add (gg->edgeedit.a, d->nearest_point, d, e);
  }

  return retval;
}

/*--------------------------------------------------------------------*/
/*                 Add and remove event handlers                      */
/*--------------------------------------------------------------------*/

void
edgeedit_event_handlers_toggle (splotd *sp, gboolean state) {
  displayd *display = sp->displayptr;
  cpaneld *cpanel = &display->cpanel;

  if (state == on) {
    sp->key_press_id = gtk_signal_connect (GTK_OBJECT (display->window),
                                           "key_press_event",
                                           (GtkSignalFunc) key_press_cb,
                                           (gpointer) sp);
    sp->press_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                       "button_press_event",
                                       (GtkSignalFunc) button_press_cb,
                                       (gpointer) sp);
    sp->release_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                         "button_release_event",
                                         (GtkSignalFunc) button_release_cb,
                                         (gpointer) sp);
    if (cpanel->ee_adding_p) {
      sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                          "motion_notify_event",
                                          (GtkSignalFunc) motion_notify_cb,
                                          (gpointer) sp);
    }

  } else {
    disconnect_key_press_signal (sp);
    disconnect_button_press_signal (sp);
    disconnect_button_release_signal (sp);
    disconnect_motion_signal (sp);
  }
}

void
cpanel_edgeedit_make (ggobid *gg) {
  GtkWidget *btn;
  GtkWidget *hb, *radio1, *radio2;
  GSList *group;
  
  gg->control_panel[EDGEED] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[EDGEED]), 5);

 /*-- Radio group in a box: add/delete buttons --*/
  hb = gtk_hbox_new (true, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hb), 3);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[EDGEED]), hb, false, false, 0);

  radio1 = gtk_radio_button_new_with_label (NULL, "Add");
  gtk_widget_set_name (radio1, "EDGEEDIT:add_radio_button");
  GTK_TOGGLE_BUTTON (radio1)->active = true;

  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio1,
    "Add new edges using the mouse", NULL);
  gtk_signal_connect (GTK_OBJECT (radio1), "toggled",
                      GTK_SIGNAL_FUNC (addordelete_cb), gg);
  gtk_box_pack_start (GTK_BOX (hb), radio1, false, false, 0);

  group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio1));

  radio2 = gtk_radio_button_new_with_label (group, "Delete");
  gtk_widget_set_name (radio2, "EDGEEDIT:delete_radio_button");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio2,
    "Delete edges using the mouse", NULL);
  gtk_box_pack_start (GTK_BOX (hb), radio2, false, false, 0);


 /*-- Undo --*/
  btn = gtk_button_new_with_label ("Undo");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Undo last action", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[EDGEED]),
                      btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (undo_last_cb), NULL);

  gtk_widget_show_all (gg->control_panel[EDGEED]);
}

/*--------------------------------------------------------------------*/
/*                      Control panel section                         */
/*--------------------------------------------------------------------*/

void
cpanel_edgeedit_init (cpaneld *cpanel, ggobid *gg) {

  cpanel->ee_adding_p = true;
  cpanel->ee_deleting_p = false;
}

void
cpanel_edgeedit_set (cpaneld *cpanel, ggobid *gg) {
  GtkWidget *w;

  /*-- set the Drag or Click radio buttons --*/
  if (cpanel->ee_adding_p) {
    w = widget_find_by_name (gg->control_panel[EDGEED],
                             "EDGEEDIT:add_radio_button");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), true);
  } else {
    w = widget_find_by_name (gg->control_panel[EDGEED],
                             "EDGEEDIT:delete_radio_button");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), true);
  }
}

