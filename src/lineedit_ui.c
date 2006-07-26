/* lineedit_ui.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include <stdlib.h>

#include "vars.h"
#include "externs.h"

static gchar *tip_edges =
  "Click and drag between\npoints to add edges.\nRight-click and drag\nfor more options.";
static gchar *tip_points =
  "Click to add points.\nRight-click for more\noptions.";

/*--------------------------------------------------------------------*/
/*                 Dialog for adding records                          */
/*--------------------------------------------------------------------*/

static void
add_record_dialog_cancel (GtkWidget * w, ggobid * gg)
{
  GtkWidget *dialog = gtk_widget_get_toplevel (w);
  gboolean rval = false;

  gg->edgeedit.a = -1;
  gtk_widget_destroy (dialog);
  edgeedit_event_handlers_toggle (gg->current_splot, true);

  g_signal_emit_by_name (G_OBJECT (gg->current_splot->da),
                         "expose_event", (gpointer) gg, (gpointer) & rval);
}

static void
add_record_dialog_apply (GtkWidget * w, displayd * display)
{
  gint j;
  cpaneld *cpanel = &display->cpanel;
  GGobiStage *d = display->d;
  GGobiStage *e = display->e;
  ggobid *gg = d->gg;
  GtkWidget *dialog = gtk_widget_get_toplevel (w);
  GtkWidget *label_entry, *id_entry;
  gchar *label = NULL, *id = NULL;
  gchar **vals = NULL;
  GGobiStage *dtarget;

  dtarget = (cpanel->ee_mode == ADDING_EDGES) ? e : d;
  if (ggobi_stage_get_n_cols(dtarget)) {
    GList *list;
    GtkTableChild *child;
    GtkWidget *entry;
    gchar *lbl;
    GtkWidget *table = widget_find_by_name (GTK_DIALOG (dialog)->vbox,
                                            "EE:tablev");

    vals = (gchar **) g_malloc (d->n_cols * sizeof (gchar *));

    for (list = GTK_TABLE (table)->children; list; list = list->next) {
      child = (GtkTableChild *) list->data;
      if (child->left_attach == 1) {
        entry = child->widget;
        lbl = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
        vals[child->top_attach] = g_strdup (lbl);
      }
    }
  }


  if ((label_entry = widget_find_by_name (GTK_DIALOG (dialog)->vbox,
                                          "EE:rowlabel"))) {
    label = gtk_editable_get_chars (GTK_EDITABLE (label_entry), 0, -1);
  }

  if ((id_entry = widget_find_by_name (GTK_DIALOG (dialog)->vbox,
                                       "EE:recordid"))) {
    id = gtk_editable_get_chars (GTK_EDITABLE (id_entry), 0, -1);
  }

  if (cpanel->ee_mode == ADDING_EDGES) {
    /*-- Add the new edge to e --*/
    record_add (cpanel->ee_mode, gg->edgeedit.a, d->nearest_point,
                label, id, vals, d, e, gg);

  }
  else if (cpanel->ee_mode == ADDING_POINTS) {
    record_add (cpanel->ee_mode, -1, -1, label, id, vals, d, e, gg);
  }

  if (vals) {
    for (j = 0; j < d->n_cols; j++)
      g_free (vals[j]);
    g_free (vals);
  }

  gg->edgeedit.a = -1;
  gtk_widget_destroy (dialog);
  edgeedit_event_handlers_toggle (gg->current_splot, true);
}

static void
add_record_dialog_open (GGobiStage * d, GGobiStage * e, displayd * dsp,
                        ggobid * gg)
{
  GtkWidget *dialog, *table;
  GtkWidget *entry, *w;
  gchar *lbl;
  cpaneld *cpanel = &dsp->cpanel;
  /*  GtkAttachOptions table_opt = GTK_SHRINK|GTK_FILL|GTK_EXPAND; */
  GtkAttachOptions table_opt = GTK_SHRINK;
  gint row = 0;
  GGobiStage *dtarget;

  edgeedit_event_handlers_toggle (gg->current_splot, false);

  if (cpanel->ee_mode == ADDING_EDGES)
    dtarget = e;
  else
    dtarget = d;

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), "Add a Record");

  table = gtk_table_new (5, 2, false);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
                      table, false, false, 5);

  w = gtk_label_new ("Record number");
  gtk_misc_set_alignment (GTK_MISC (w), 1, .5);
  gtk_table_attach (GTK_TABLE (table),
                    w, 0, 1, row, row + 1, table_opt, table_opt, 1, 1);
  lbl = g_strdup_printf ("%d", dtarget->n_rows);
  w = gtk_label_new (lbl);
  gtk_misc_set_alignment (GTK_MISC (w), .5, .5);
  gtk_table_attach (GTK_TABLE (table),
                    w, 1, 2, row, row + 1, table_opt, table_opt, 1, 1);
  g_free (lbl);
  row++;

  if (cpanel->ee_mode == ADDING_EDGES) {
    w = gtk_label_new ("Edge source");
    gtk_misc_set_alignment (GTK_MISC (w), 1, .5);
    gtk_table_attach (GTK_TABLE (table),
                      w, 0, 1, row, row + 1, table_opt, table_opt, 1, 1);
    /* This label should include both the rowlab and the rowId */
    lbl = ggobi_stage_get_row_id(d, gg->edgeedit.a);
    w = gtk_label_new (lbl);
    gtk_misc_set_alignment (GTK_MISC (w), .5, .5);
    gtk_table_attach (GTK_TABLE (table),
                      w, 1, 2, row, row + 1, table_opt, table_opt, 1, 1);
    row++;

    w = gtk_label_new ("Edge destination");
    gtk_misc_set_alignment (GTK_MISC (w), 1, .5);
    gtk_table_attach (GTK_TABLE (table),
                      w, 0, 1, row, row + 1, table_opt, table_opt, 1, 1);
    lbl = ggobi_stage_get_row_id(d, d->nearest_point);
    w = gtk_label_new (lbl);
    gtk_misc_set_alignment (GTK_MISC (w), .5, .5);
    gtk_table_attach (GTK_TABLE (table),
                      w, 1, 2, row, row + 1, table_opt, table_opt, 1, 1);
    row++;
  }

  w = gtk_label_new_with_mnemonic ("Record _label");
  gtk_misc_set_alignment (GTK_MISC (w), 1, .5);
  gtk_table_attach (GTK_TABLE (table),
                    w, 0, 1, row, row + 1, table_opt, table_opt, 1, 1);
  entry = gtk_entry_new ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (w), entry);
  lbl = g_strdup_printf ("%d", dtarget->n_rows + 1);
  gtk_entry_set_text (GTK_ENTRY (entry), lbl);
  g_free (lbl);

  gtk_widget_set_name (entry, "EE:rowlabel");
  gtk_table_attach (GTK_TABLE (table),
                    entry, 1, 2, row, row + 1, table_opt, table_opt, 1, 1);
  row++;

  if ((cpanel->ee_mode == ADDING_POINTS) ||
      (cpanel->ee_mode == ADDING_EDGES)) {
    w = gtk_label_new_with_mnemonic ("Record _id");
    gtk_misc_set_alignment (GTK_MISC (w), 1, .5);
    gtk_table_attach (GTK_TABLE (table),
                      w, 0, 1, row, row + 1, table_opt, table_opt, 1, 1);
    entry = gtk_entry_new ();
    gtk_label_set_mnemonic_widget (GTK_LABEL (w), entry);
    lbl = g_strdup_printf ("%d", dtarget->n_rows + 1);
    gtk_entry_set_text (GTK_ENTRY (entry), lbl);
    g_free (lbl);
    gtk_widget_set_name (entry, "EE:recordid");
    gtk_table_attach (GTK_TABLE (table),
                      entry, 1, 2, row, row + 1, table_opt, table_opt, 1, 1);
    row++;
  }

  /*-- Another table to contain variable name-value pairs --*/
  if (ggobi_stage_get_n_cols(dtarget)) {
    gint j;
    GtkWidget *tablev;
    gchar **vals = (gchar **) g_malloc (dtarget->n_cols * sizeof (gchar *));

    extern void fetch_default_record_values (gchar ** vals,
                                             GGobiStage *, displayd *,
                                             ggobid * gg);
    fetch_default_record_values (vals, dtarget, dsp, gg);

    tablev = gtk_table_new (dtarget->n_cols, 2, false);
    gtk_widget_set_name (tablev, "EE:tablev");
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
                        tablev, false, false, 5);

    for (j = 0; j < dtarget->n_cols; j++) {
      w = gtk_label_new (ggobi_stage_get_col_name(d, j));
      gtk_table_attach (GTK_TABLE (tablev),
                        w, 0, 1, j, j + 1, table_opt, table_opt, 1, 1);

      entry = gtk_entry_new ();
      gtk_entry_set_text (GTK_ENTRY (entry), vals[j]);
      gtk_table_attach (GTK_TABLE (tablev),
                        entry, 1, 2, j, j + 1, table_opt, table_opt, 1, 1);
    }

    /* free vals, I think */
    for (j = 0; j < dtarget->n_cols; j++)
      g_free (vals[j]);
    g_free (vals);
  }

  /*-- ok button --*/
  w = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), w,
                        "Add the point or edge.  To avoid seeing this dialog, use the left button.",
                        NULL);
  g_signal_connect (G_OBJECT (w), "clicked",
                    G_CALLBACK (add_record_dialog_apply), dsp);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area), w);

  /*-- cancel button --*/
  w = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  g_signal_connect (G_OBJECT (w), "clicked",
                    G_CALLBACK (add_record_dialog_cancel), gg);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area), w);


  gtk_widget_show_all (dialog);
}




/*--------------------------------------------------------------------*/
/*          Respond to buttons and menus in the panel                 */
/*--------------------------------------------------------------------*/

static void
add_edges_or_points_cb (GtkToggleButton * button, ggobid * gg)
{
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;
  GtkWidget *panel =
    mode_panel_get_by_name (ggobi_getIModeName (EDGEED), gg);
  GtkWidget *w;

  w = widget_find_by_name (panel, "EDGEEDIT:tip_label");

  if (button->active) {
    cpanel->ee_mode = ADDING_EDGES;
    splot_cursor_set ((gint) NULL, gg->current_splot);
    gtk_label_set_text (GTK_LABEL (w), tip_edges);
  }
  else {
    cpanel->ee_mode = ADDING_POINTS;
    splot_cursor_set (GDK_CROSSHAIR, gg->current_splot);
    gtk_label_set_text (GTK_LABEL (w), tip_points);
  }
}

/*--------------------------------------------------------------------*/
/*          Handling and mouse events in the plot window              */
/*--------------------------------------------------------------------*/

static gint
key_press_cb (GtkWidget * w, GdkEventKey * event, splotd * sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  cpaneld *cpanel = &gg->current_display->cpanel;

  /*-- handle the keys for setting the mode and launching generic events --*/
  if (splot_event_handled (w, event, cpanel, sp, gg))
    return true;

  /*-- insert mode-specific key presses (if any) here --*/

  return false;
}

static gint
motion_notify_cb (GtkWidget * w, GdkEventMotion * event, splotd * sp)
{
  displayd *display = sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  GGobiStage *d = display->d;
  ggobid *gg = GGobiFromSPlot (sp);
  gboolean button1_p, button2_p;
  gint k;

  if (cpanel->ee_mode == ADDING_EDGES) {
    mousepos_get_motion (w, event, &button1_p, &button2_p, sp);
    k = find_nearest_point (&sp->mousepos, sp, d, gg);
    d->nearest_point = k;

    if (k != d->nearest_point_prev) {

      if (gg->edgeedit.a == -1) {  /*-- looking for starting point --*/
        if (k != d->nearest_point_prev)
          displays_plot (NULL, QUICK, gg);
      }
      else {    /*-- found starting point; looking for ending point --*/

        displays_plot (NULL, QUICK, gg);
        /*-- add a dotted line from gg->edgeedit.a to gg->nearest_point --*/
      }
    }
    d->nearest_point_prev = d->nearest_point;

  }
  else if (cpanel->ee_mode == ADDING_POINTS) {
    ;
  }

  return true;
}

static gint
button_press_cb (GtkWidget * w, GdkEventButton * event, splotd * sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  displayd *display = sp->displayptr;
  GGobiStage *d = display->d;
  gboolean button1_p, button2_p;
  mousepos_get_pressed (w, event, &button1_p, &button2_p, sp);

  gg->current_splot = sp->displayptr->current_splot = sp;
  gg->current_display = display;

  gg->edgeedit.a = d->nearest_point;

  return true;
}

static gint
button_release_cb (GtkWidget * w, GdkEventButton * event, gpointer user_data)
{
  gboolean retval = true;
  splotd *sp = GGOBI_SPLOT(user_data);
  ggobid *gg = GGobiFromSPlot (sp);
  displayd *display = sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  GGobiStage *d = display->d;
  GGobiStage *e = display->e;
  gint which_button = 1;

  void record_add_defaults (GGobiStage * d, GGobiStage * e, displayd * display,
                            ggobid * gg);

  if ((event->state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
    which_button = 1;
  else if ((event->state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
    which_button = 2;
  else if ((event->state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
    which_button = 2;

  gg->buttondown = 0;

  sp->mousepos.x = (gint) event->x;
  sp->mousepos.y = (gint) event->y;

  /*
   * add the edge to display->e.  If display->e is NULL, then
   * initialize it first.
   */
  if (cpanel->ee_mode == ADDING_EDGES) {
    if (d->nearest_point >= 0 &&
        gg->edgeedit.a >= 0 && d->nearest_point != gg->edgeedit.a) {

      if (e == NULL) {
        /*-- initialize e, the new datad --*/
        e = GGOBI_STAGE(ggobi_data_new (0, 0));
        ggobi_stage_set_name(e, "edges");
        display->e = e;
        display->options.edges_undirected_show_p = true;
      }

      if (which_button == 1)
        record_add_defaults (d, e, display, gg);
      else
        /*-- Open a dialog window to ask for label, rowId, data ... --*/
        add_record_dialog_open (d, e, display, gg);
    }

  }
  else if (cpanel->ee_mode == ADDING_POINTS) {
    if (which_button == 1)
      record_add_defaults (d, e, display, gg);
    else
      /*-- Open a dialog window to ask for label, rowId, data ... --*/
      add_record_dialog_open (d, e, display, gg);
  }

  /*-- Release the pointer so the button press can be detected --*/
  gdk_pointer_ungrab (event->time);
  return retval;
}

/*--------------------------------------------------------------------*/
/*                 Add and remove event handlers                      */
/*--------------------------------------------------------------------*/

void
edgeedit_event_handlers_toggle (splotd * sp, gboolean state)
{
  displayd *display = sp->displayptr;

  if (state == on) {
    if (GGOBI_IS_WINDOW_DISPLAY (display))
      sp->key_press_id =
        g_signal_connect (G_OBJECT (GGOBI_WINDOW_DISPLAY (display)->window),
                          "key_press_event", G_CALLBACK (key_press_cb),
                          (gpointer) sp);
    sp->press_id =
      g_signal_connect (G_OBJECT (sp->da), "button_press_event",
                        G_CALLBACK (button_press_cb), (gpointer) sp);
    sp->release_id =
      g_signal_connect (G_OBJECT (sp->da), "button_release_event",
                        G_CALLBACK (button_release_cb), (gpointer) sp);
    sp->motion_id =
      g_signal_connect (G_OBJECT (sp->da), "motion_notify_event",
                        G_CALLBACK (motion_notify_cb), (gpointer) sp);

  }
  else {
    disconnect_key_press_signal (sp);
    disconnect_button_press_signal (sp);
    disconnect_button_release_signal (sp);
    disconnect_motion_signal (sp);
  }
}

void
cpanel_edgeedit_make (ggobid * gg)
{
  modepaneld *panel;
  GtkWidget *vb, *radio1, *radio2, *w;
  GSList *group;
  // This should be in an init routine
  gboolean adding_edges = true;

  panel = (modepaneld *) g_malloc (sizeof (modepaneld));
  gg->control_panels = g_list_append (gg->control_panels, (gpointer) panel);
  panel->name = g_strdup (ggobi_getIModeName (EDGEED));
  panel->w = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (panel->w), 5);

 /*-- Radio group in a box: add edges or points buttons --*/
  vb = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 3);
  gtk_box_pack_start (GTK_BOX (panel->w), vb, false, false, 0);

  radio1 = gtk_radio_button_new_with_mnemonic (NULL, "Add _edges");
  gtk_widget_set_name (radio1, "EDGEEDIT:add_edges_radio_button");
  if (adding_edges)
    GTK_TOGGLE_BUTTON (radio1)->active = true;

  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio1,
                        "Add new edges using the mouse. The right or middle button opens a dialog window; the left button adds an edge using defaults.",
                        NULL);
  g_signal_connect (G_OBJECT (radio1), "toggled",
                    G_CALLBACK (add_edges_or_points_cb), gg);
  gtk_box_pack_start (GTK_BOX (vb), radio1, false, false, 0);

  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio1));

  radio2 = gtk_radio_button_new_with_mnemonic (group, "Add _points");
  gtk_widget_set_name (radio2, "EDGEEDIT:add_points_radio_button");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio2,
                        "Add points using the mouse.  The right or button opens a dialog window; the left button adds a point using defaults.",
                        NULL);
  gtk_box_pack_start (GTK_BOX (vb), radio2, false, false, 0);

  w = gtk_label_new (tip_edges);
  gtk_label_set_line_wrap (GTK_LABEL (w), true);
  gtk_box_pack_start (GTK_BOX (vb), w, false, false, 0);
  gtk_widget_set_name (w, "EDGEEDIT:tip_label");

  /*-- Undo --*/
  /*   not implemented
     btn = gtk_button_new_with_label ("Undo");
     gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
     "Undo last action", NULL);
     gtk_box_pack_start (GTK_BOX (panel->w),
     btn, false, false, 1);
     g_signal_connect (G_OBJECT (btn), "clicked",
     G_CALLBACK (undo_last_cb), NULL);
   */

  gtk_widget_show_all (panel->w);
}

/*--------------------------------------------------------------------*/
/*                      Control panel section                         */
/*--------------------------------------------------------------------*/

void
cpanel_edgeedit_init (cpaneld * cpanel, ggobid * gg)
{
  cpanel->ee_mode = ADDING_EDGES;
}

void
cpanel_edgeedit_set (displayd * display, cpaneld * cpanel, ggobid * gg)
{
  GtkWidget *w;
  GtkWidget *panel =
    mode_panel_get_by_name (ggobi_getIModeName (EDGEED), gg);

  GtkWidget *lbl = widget_find_by_name (panel, "EDGEEDIT:tip_label");

  /*-- set the Drag or Click radio buttons --*/
  if (cpanel->ee_mode == ADDING_EDGES) {
    w = widget_find_by_name (panel, "EDGEEDIT:add_edges_radio_button");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), true);
    gtk_label_set_text (GTK_LABEL (lbl), tip_edges);
  }
  else {
    w = widget_find_by_name (panel, "EDGEEDIT:add_points_radio_button");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), true);
    gtk_label_set_text (GTK_LABEL (lbl), tip_points);
  }
}

/*--------------------------------------------------------------------*/

RedrawStyle
edgeedit_activate (gboolean state, displayd * display, ggobid * gg)
{
  cpaneld *cpanel = &display->cpanel;
  RedrawStyle redraw_style = QUICK;

  if (state) {
    if (cpanel->ee_mode == ADDING_POINTS)
      splot_cursor_set (GDK_CROSSHAIR, gg->current_splot);
  }
  else
    splot_cursor_set ((gint) NULL, gg->current_splot);

  return redraw_style;
}
