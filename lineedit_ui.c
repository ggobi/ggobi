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
/*                 Dialog for adding records                          */
/*--------------------------------------------------------------------*/

static void
add_record_dialog_cancel (GtkWidget *w, ggobid *gg) 
{
  GtkWidget *dialog = gtk_widget_get_toplevel (w);
  gtk_widget_destroy (dialog);
}

static void
add_record_dialog_apply (GtkWidget *w, ggobid *gg) 
{
  g_printerr ("add the record\n");
}

static void
add_record_dialog_open (datad *d, datad *e, displayd *dsp, ggobid *gg)
{
  GtkWidget *dialog, *table;
  GtkWidget *label, *entry, *w;
  gchar *lbl;
  cpaneld *cpanel = &dsp->cpanel;
  GtkAttachOptions table_opt = GTK_SHRINK|GTK_FILL|GTK_EXPAND;

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW(dialog), "Add a record");

  table = gtk_table_new (2, 3, false);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), table);

  w = gtk_label_new ("Record number");
  gtk_misc_set_alignment (GTK_MISC (w), 1, .5);
  gtk_table_attach (GTK_TABLE (table),
    w, 0, 1, 0, 1, /* left right top bottom */
    table_opt, table_opt, 1, 1);
  lbl = g_strdup_printf ("%d",
    (cpanel->ee_adding_edges_p)?e->nrows:d->nrows + 1);
  w = gtk_label_new (lbl);
  gtk_misc_set_alignment (GTK_MISC (w), .5, .5);
  gtk_table_attach (GTK_TABLE (table),
    w, 1, 2, 0, 1, table_opt, table_opt, 1, 1);
  g_free (lbl);

  w = gtk_label_new ("Record label");
  gtk_misc_set_alignment (GTK_MISC (w), 1, .5);
  gtk_table_attach (GTK_TABLE (table),
    w, 0, 1, 1, 2, table_opt, table_opt, 1, 1);
  entry = gtk_entry_new ();
  gtk_widget_set_name (entry, "EE:rowlabel");
  gtk_table_attach (GTK_TABLE (table),
    entry, 1, 2, 1, 2, table_opt, table_opt, 1, 1);

  w = gtk_label_new ("Record id");
  gtk_misc_set_alignment (GTK_MISC (w), 1, .5);
  gtk_table_attach (GTK_TABLE (table),
    w, 0, 1, 2, 3, table_opt, table_opt, 1, 1);
  entry = gtk_entry_new ();
  gtk_widget_set_name (entry, "EE:recordid");
  gtk_table_attach (GTK_TABLE (table),
    entry, 1, 2, 2, 3,
    table_opt, table_opt, 1, 1);

  /*-- ok button --*/
  w = gtk_button_new_with_label ("Apply");
  gtk_signal_connect (GTK_OBJECT (w), "clicked",
    GTK_SIGNAL_FUNC (add_record_dialog_apply), dsp);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area), w);

  /*-- cancel button --*/
  w = gtk_button_new_with_label ("Close");
  gtk_signal_connect (GTK_OBJECT (w), "clicked",
    GTK_SIGNAL_FUNC (add_record_dialog_cancel), dsp);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area), w);

  gtk_widget_show_all (dialog);
}




/*--------------------------------------------------------------------*/
/*          Respond to buttons and menus in the panel                 */
/*--------------------------------------------------------------------*/

static void add_edges_or_points_cb (GtkToggleButton *button, ggobid *gg)
{
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;

  cpanel->ee_adding_edges_p = button->active;
  cpanel->ee_adding_points_p = !button->active;
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

  if (cpanel->ee_adding_edges_p && k != d->nearest_point_prev) {
    if (gg->edgeedit.a == -1) {  /*-- looking for starting point --*/
      if (k != d->nearest_point_prev)
        displays_plot (NULL, QUICK, gg);
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

  gg->current_splot = sp->displayptr->current_splot = sp;
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
  cpaneld *cpanel = &display->cpanel;
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
  if (d->nearest_point >= 0) {
    g_printerr ("add the edge from %d to %d\n",
      gg->edgeedit.a, d->nearest_point);

    if (e == NULL) {
      /*-- Initialize a new edge set --*/
      g_printerr ("Not yet initializing a new edge set\n");
      return false;
    }

    if (e->ncols) {
      g_printerr ("Not yet adding edges to datad's with variables\n");
      return false;
    }

    if (d->rowIds == NULL) {
      /*-- Add rowids to d --*/
      g_printerr ("Not yet initializing new rowids\n");
      return false;
    }

    /*-- Open a dialog window to ask for label and maybe rowId --*/
    /*-- just use defaults for the moment --*/

    /*-- Add the new edge to e --*/
/*

d:  We're not making any changes to d when we add an edge.

e:
  record label
  if e has rowIds, a rowId
  if e has variables, variable values -- we don't have a clue what to use
*/

/*
    edge_add (gg->edgeedit.a, d->nearest_point, NULL, NULL, d, e, gg);
*/

    add_record_dialog_open (d, e, display, gg);

  }
  gg->edgeedit.a = -1;

  /*-- Release the pointer so the button press can be detected --*/
  gdk_pointer_ungrab (event->time);
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
    if(GTK_IS_GGOBI_WINDOW_DISPLAY(display))
      sp->key_press_id = gtk_signal_connect (GTK_OBJECT (GTK_GGOBI_WINDOW_DISPLAY(display)->window),
        "key_press_event", (GtkSignalFunc) key_press_cb, (gpointer) sp);
    sp->press_id = gtk_signal_connect (GTK_OBJECT (sp->da),
      "button_press_event", (GtkSignalFunc) button_press_cb, (gpointer) sp);
    sp->release_id = gtk_signal_connect (GTK_OBJECT (sp->da),
      "button_release_event", (GtkSignalFunc) button_release_cb, (gpointer) sp);
    sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
      "motion_notify_event", (GtkSignalFunc) motion_notify_cb, (gpointer) sp);

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

 /*-- Radio group in a box: add edges or points buttons --*/
  hb = gtk_vbox_new (true, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hb), 3);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[EDGEED]), hb, false, false, 0);

  radio1 = gtk_radio_button_new_with_label (NULL, "Add edges");
  gtk_widget_set_name (radio1, "EDGEEDIT:add_edges_radio_button");
  GTK_TOGGLE_BUTTON (radio1)->active = true;

  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio1,
    "Add new edges using the mouse", NULL);
  gtk_signal_connect (GTK_OBJECT (radio1), "toggled",
                      GTK_SIGNAL_FUNC (add_edges_or_points_cb), gg);
  gtk_box_pack_start (GTK_BOX (hb), radio1, false, false, 0);

  group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio1));

  radio2 = gtk_radio_button_new_with_label (group, "Add points");
  gtk_widget_set_name (radio2, "EDGEEDIT:add_points_radio_button");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio2,
    "Add points using the mouse", NULL);
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
cpanel_edgeedit_init (cpaneld *cpanel, ggobid *gg)
{
  cpanel->ee_adding_edges_p = true;
  cpanel->ee_adding_points_p = false;
}

void
cpanel_edgeedit_set (cpaneld *cpanel, ggobid *gg) {
  GtkWidget *w;

  /*-- set the Drag or Click radio buttons --*/
  if (cpanel->ee_adding_edges_p) {
    w = widget_find_by_name (gg->control_panel[EDGEED],
                             "EDGEEDIT:add_edges_radio_button");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), true);
  } else {
    w = widget_find_by_name (gg->control_panel[EDGEED],
                             "EDGEEDIT:add_points_radio_button");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), true);
  }
}

