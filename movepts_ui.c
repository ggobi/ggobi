/* movepts_ui.c */
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

static void reset_all_cb (GtkButton *button, ggobid *gg)
{
  GSList *l;
  datad *d;
  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;

    while (g_slist_length (d->movepts_history) > 0) {
      movepts_history_delete_last (d, gg);
      movepts_history_delete_last (d, gg);
    }
    tform_to_world (d, gg);
  }

  displays_tailpipe (REDISPLAY_ALL, gg);
}

static void undo_last_cb (GtkButton *button, ggobid *gg)
{
  datad *d = gg->current_display->d;

/*-- remove the last two cells --*/
  movepts_history_delete_last (d, gg);
  movepts_history_delete_last (d, gg);
  tform_to_world (d, gg);
  displays_tailpipe (REDISPLAY_ALL, gg);
}

static void move_cluster_cb (GtkWidget *w, ggobid *gg)
{
  gg->movepts.cluster_p = GTK_TOGGLE_BUTTON (w)->active;
}

static gchar *mdir_lbl[] = {"Both", "Vertical", "Horizontal"};
static void mdir_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  gg->movepts.direction = (enum directiond) GPOINTER_TO_INT (cbd);
}

/*--------------------------------------------------------------------*/
/*      Handling keyboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

static gint
key_press_cb (GtkWidget *w, GdkEventKey *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot(sp);
  cpaneld *cpanel = &gg->current_display->cpanel;
  
/*-- add a key_press_cb in each mode, and let it begin with these lines --*/
  if (scatterplot_event_handled (w, event, cpanel, sp, gg))
    return true;

  /*-- insert mode-specific key presses (if any) here --*/

  return true;
}

static gint
motion_notify_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  displayd *display;
  datad *d;
  gboolean button1_p, button2_p;
  gboolean inwindow, wasinwindow, pointer_moved;

  gg->current_splot = sp;
  display = (displayd *) sp->displayptr;
  d = display->d;

  /*-- try defining wasinwindow before the new mousepos is calculated --*/
  wasinwindow = mouseinwindow (sp);

  /*-- get the mouse position and find out which buttons are pressed --*/
  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);

  inwindow = mouseinwindow (sp);

  /*-- If the pointer is inside the plotting region ... --*/
  if (inwindow) {
    /*-- If the pointer has moved ...--*/
    if ((sp->mousepos.x != sp->mousepos_o.x) ||
        (sp->mousepos.y != sp->mousepos_o.y))
    {
      pointer_moved = true;
      sp->mousepos_o.x = sp->mousepos.x;
      sp->mousepos_o.y = sp->mousepos.y;
    }
  }

  if (pointer_moved) {
    
    /*
     * If the left button is down, move the point: compute the
     * data pipeline in reverse, (then run it forward again?) and
     * draw the plot.
    */
    if (d->nearest_point != -1) {
      move_pt (d->nearest_point,
               sp->mousepos.x,
               sp->mousepos.y,
               sp, d, gg);
    }
  }

  if (!inwindow && wasinwindow) {
    d->nearest_point = -1;
    splot_redraw (sp, QUICK, gg);  
  }

  return true;
}


static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  ggobid *gg = GGobiFromSPlot (sp);
  datad *d = gg->current_display->d;
  
  gg->current_display = (displayd *) sp->displayptr;
  gg->current_splot = sp;

  /*
   * allow point motion only for
   *   scatterplots in xyplot mode
   *   the splotd members of a scatmat that are xyplots.
  */
  if ((display->displaytype == scatterplot && cpanel->projection == XYPLOT) ||
      (display->displaytype == scatmat && sp->p1dvar == -1))
  {
    sp->mousepos.x = (gint) event->x;
    sp->mousepos.y = (gint) event->y;
    sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                        "motion_notify_event",
                                        (GtkSignalFunc) motion_notify_cb,
                                        (gpointer) sp);
    d->nearest_point = find_nearest_point (&sp->mousepos, sp, d, gg);
    if (d->nearest_point != -1) {
      movepts_history_add (d->nearest_point, sp, d, gg);

      /*-- add the history information for the cluster here --*/
      if (gg->movepts.cluster_p) {
        clusters_set (d, gg);
        if (d->nclusters > 1) {
          gint i, k, id = d->nearest_point;
          gfloat cur_clust = d->clusterids.els[id];
          for (i=0; i<d->nrows_in_plot; i++) {
            k = d->rows_in_plot[i];
            if (k == id)
              ;
            else
              if (d->clusterids.els[k] == cur_clust)
                if (!d->hidden_now.els[k])
                  movepts_history_add (k, sp, d, gg);
          }
        }
      }

      splot_redraw (sp, QUICK, gg);  
    }
  } else {
    g_printerr ("You can only move points in xyplot mode\n");
  }

  return true;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  ggobid *gg = GGobiFromSPlot (sp);

  gg->current_splot = sp;

  if (cpanel->projection == XYPLOT) {
    sp->mousepos.x = (gint) event->x;
    sp->mousepos.y = (gint) event->y;
    if (sp->motion_id)
      gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->motion_id);
  }

  return retval;
}

void
movepts_event_handlers_toggle (splotd *sp, gboolean state) {
  displayd *display = (displayd *) sp->displayptr;

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
  } else {
    if (sp->key_press_id)
      gtk_signal_disconnect (GTK_OBJECT (display->window), sp->key_press_id);
    if (sp->press_id)
      gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->press_id);
    if (sp->release_id)
      gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->release_id);
  }
}

void
cpanel_movepts_make (ggobid *gg) {
  GtkWidget *btn, *opt, *box, *hb, *lbl;
  
  gg->control_panel[MOVEPTS] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[MOVEPTS]), 5);

/*
 * option menu: direction of motion 
*/
  hb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[MOVEPTS]),
                      hb, false, false, 0);

  lbl = gtk_label_new ("Direction of motion:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 1);
  gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Move freely, or constrain the motion vertically or horizontally",
    NULL);
  populate_option_menu (opt, mdir_lbl,
                        sizeof (mdir_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) mdir_cb, gg);
  gtk_box_pack_start (GTK_BOX (hb), opt, false, false, 0);

/*
 * Use group toggle
*/
  btn = gtk_check_button_new_with_label ("Move brush group");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Move all points with the same symbol", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "toggled",
                     GTK_SIGNAL_FUNC (move_cluster_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[MOVEPTS]),
    btn, false, false, 1);

/*
 * Box to hold reset buttons
*/
  box = gtk_hbox_new (true, 2);

  btn = gtk_button_new_with_label ("Reset all");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Reset all points to their original positions", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                     GTK_SIGNAL_FUNC (reset_all_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, false, false, 1);

  btn = gtk_button_new_with_label ("Undo last");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Undo the previous move", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                     GTK_SIGNAL_FUNC (undo_last_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, false, false, 1);

  gtk_box_pack_start (GTK_BOX (gg->control_panel[MOVEPTS]),
                      box, false, false, 1);

  gtk_widget_show_all (gg->control_panel[MOVEPTS]);
}
