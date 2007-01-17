/* movepts_ui.c */
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

#include "vars.h"
#include "externs.h"


static void
reset_all_cb (GtkButton * button, GGobiSession * gg)
{
  //GSList *l;
  GGobiStage *d;
  /* do we really want reset over all the datasets?? - mfl */
  //for (l = gg->d; l; l = l->next) {
    d = (GGobiStage *) gg->current_display->d;

    while (g_slist_length (d->movepts_history) > 0) {
      /*-- yes, twice -- once for x motion, once for y motion --*/
      movepts_history_delete_last (d, gg);
      movepts_history_delete_last (d, gg);
    }
    tform_to_world(d);
  //}

  displays_tailpipe (FULL, gg);
}

static void
undo_last_cb (GtkButton * button, GGobiSession * gg)
{
  GGobiStage *d = gg->current_display->d;

/*-- remove the last two cells --*/
  movepts_history_delete_last (d, gg);
  movepts_history_delete_last (d, gg);
  tform_to_world(d);
  displays_tailpipe (FULL, gg);
}

static void
move_cluster_cb (GtkWidget * w, GGobiSession * gg)
{
  gg->movepts.cluster_p = GTK_TOGGLE_BUTTON (w)->active;
}

static gchar *mdir_lbl[] = { "Both", "Vertical", "Horizontal" };
static void
mdir_cb (GtkWidget * w, GGobiSession * gg)
{
  gg->movepts.direction =
    (enum directiond) gtk_combo_box_get_active (GTK_COMBO_BOX (w));
}

/*--------------------------------------------------------------------*/
/*      Handling keyboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

static gint
key_press_cb (GtkWidget * w, GdkEventKey * event, splotd * sp)
{
  GGobiSession *gg = GGobiFromSPlot (sp);
  cpaneld *cpanel = &gg->current_display->cpanel;

/*-- add a key_press_cb in each mode, and let it begin with these lines --*/
  if (splot_event_handled (w, event, cpanel, sp, gg))
    return true;

  /*-- insert mode-specific key presses (if any) here --*/

  return false;
}

static gint
motion_notify_cb (GtkWidget * w, GdkEventMotion * event, splotd * sp)
{
  GGobiSession *gg = GGobiFromSPlot (sp);
  displayd *display = sp->displayptr;

  gg->current_splot = sp->displayptr->current_splot = sp;/*-- just in case --*/

  /*
   * allow point motion only for
   *   scatterplots 
   *   the splotd members of a scatmat that are xyplots.
   */
  if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
    GGobiExtendedDisplayClass *klass;
    klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
    if (klass->move_points_motion_cb)
      klass->move_points_motion_cb (display, sp, w, event, gg);
  }

  return true;
}


static gint
button_press_cb (GtkWidget * w, GdkEventButton * event, splotd * sp)
{
  displayd *display = (displayd *) sp->displayptr;
  GGobiSession *gg = GGobiFromSPlot (sp);

  gg->current_display = display;
  gg->current_splot = sp->displayptr->current_splot = sp;

  /*
   * allow point motion only for
   *   scatterplots  
   *   the splotd members of a scatmat that are xyplots.
   */
  if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
    GGobiExtendedDisplayClass *klass;
    klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
    if (klass->move_points_button_cb)
      klass->move_points_button_cb (display, sp, w, event, gg);
  }
  else
    g_printerr ("Sorry, you can not points in this display or plot\n");

  return true;
}

static gint
button_release_cb (GtkWidget * w, GdkEventButton * event, splotd * sp)
{
  gboolean retval = true;
  GGobiSession *gg = GGobiFromSPlot (sp);

  gg->buttondown = 0;

  gdk_pointer_ungrab (event->time);  /*-- grabbed in mousepos_get_pressed --*/

  displays_plot (NULL, QUICK, gg);

  return retval;
}

void
movepts_event_handlers_toggle (splotd * sp, gboolean state)
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
cpanel_movepts_make (GGobiSession * gg)
{
  modepaneld *panel;
  GtkWidget *btn, *opt, *box, *hb, *lbl;

  panel = (modepaneld *) g_malloc (sizeof (modepaneld));
  gg->control_panels = g_list_append (gg->control_panels, (gpointer) panel);
  panel->name = g_strdup (ggobi_getIModeName (MOVEPTS));

  panel->w = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (panel->w), 5);

  /*-- option menu: direction of motion --*/
  hb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (panel->w), hb, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("Direction of _motion:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 1);
  gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);

  opt = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), opt);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
                        "Move freely, or constrain the motion vertically or horizontally",
                        NULL);
  populate_combo_box (opt, mdir_lbl, G_N_ELEMENTS (mdir_lbl),
                      G_CALLBACK (mdir_cb), gg);
  gtk_box_pack_start (GTK_BOX (hb), opt, false, false, 0);

  /*-- Use group toggle --*/
  btn = gtk_check_button_new_with_mnemonic ("Move brush _group");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                        "Move all points with the same symbol", NULL);
  g_signal_connect (G_OBJECT (btn), "toggled",
                    G_CALLBACK (move_cluster_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (panel->w), btn, false, false, 1);

  /*-- Box to hold reset buttons --*/
  box = gtk_hbox_new (true, 2);

  btn = gtk_button_new_with_mnemonic ("_Reset all");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                        "Reset all points to their original positions", NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
                    G_CALLBACK (reset_all_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, false, false, 1);

  btn = gtk_button_new_from_stock (GTK_STOCK_UNDO);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                        "Undo the previous move", NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
                    G_CALLBACK (undo_last_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, false, false, 1);

  gtk_box_pack_start (GTK_BOX (panel->w), box, false, false, 1);

  gtk_widget_show_all (panel->w);
}
