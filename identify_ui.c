/*-- identify_ui.c --*/
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

static void display_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->identify_display_type = GPOINTER_TO_INT (cbd);
  displays_plot (NULL, QUICK, gg);
}

static void
id_remove_labels_cb (GtkWidget *w, ggobid *gg)
{
  datad *d = gg->current_display->d;
  g_slist_free (d->sticky_ids);
  d->sticky_ids = (GSList *) NULL;
  displays_plot (NULL, QUICK, gg);
}
static void
id_all_sticky_cb (GtkWidget *w, ggobid *gg)
{
  gint i, m;
  datad *d = gg->current_display->d;

  /*-- clear the list before adding to avoid redundant entries --*/
  g_slist_free (d->sticky_ids);
  d->sticky_ids = (GSList *) NULL;

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    d->sticky_ids = g_slist_append (d->sticky_ids, GINT_TO_POINTER (i));
  }
  displays_plot (NULL, QUICK, gg);
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
  if (splot_event_handled (w, event, cpanel, sp, gg))
    return true;

  /*-- insert mode-specific key presses (if any) here --*/

  return true;
}

void
displays_add_point_cues (splotd *splot, gint k, ggobid *gg) {
  GList *dlist, *slist;
  displayd *display;
  splotd *sp;

  for (dlist = gg->displays; dlist; dlist = dlist->next) {
    display = (displayd *) dlist->data;
    for (slist = display->splots; slist; slist = slist->next) {
      sp = (splotd *) slist->data;
      if (sp != splot) {
        splot_redraw (sp, QUICK, gg);
      }
    }
  }
}


static gint
motion_notify_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp)
{
  gint k;
  ggobid *gg = GGobiFromSPlot(sp);
  datad *d = gg->current_display->d;
  gboolean button1_p, button2_p;
  gint nd = g_slist_length (gg->d);
  extern void identify_link_by_id (gint k, datad *source_d, ggobid *gg);

/*
 * w = sp->da
 * sp = gtk_object_get_data (GTK_OBJECT (w), "splotd"));
*/

  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);

#ifdef BARCHART_IMPLEMENTED
  if (gg->current_display->displaytype == barchart) {
    gboolean changed = barchart_identify_bars (sp->mousepos, sp, d, gg);

    if (changed) {
        displays_plot (NULL, QUICK, gg);
    }
  } else {
#endif

  k = find_nearest_point (&sp->mousepos, sp, d, gg);
  d->nearest_point = k;

  /*-- link by id --*/
  if (nd > 1 && k >= 0) identify_link_by_id (k, d, gg);
  /*-- --*/

  if (k != d->nearest_point_prev) {
    displays_plot (NULL, QUICK, gg);

    if (gg->identify_handler.handler) {
      (gg->identify_handler.handler)(gg->identify_handler.user_data,
        k, sp, w, gg);
    }

    {
	GGobiPointMoveEvent ev;
        ev.d = d;
	ev.id = k;
        gtk_signal_emit(GTK_OBJECT(w), GGobiSignals[IDENTIFY_POINT_SIGNAL], sp, &ev, gg); 
    }
    d->nearest_point_prev = k;
  }
#ifdef BARCHART_IMPLEMENTED
}
#endif
  return true;  /* no need to propagate the event */
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
/*
 * If nearest_point is a member of gg->sticky_ids, remove it; if
 * it isn't, add it.
*/
  ggobid *gg = GGobiFromSPlot(sp);
  displayd *display = sp->displayptr;
  datad *d = display->d;
  extern void sticky_id_toggle (datad *, ggobid *);
  sticky_id_toggle (d, gg);

  return true;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  gg->buttondown = 0;
  return true;
}

void
identify_event_handlers_toggle (splotd *sp, gboolean state) {
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
    sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                        "motion_notify_event",
                                        (GtkSignalFunc) motion_notify_cb,
                                        (gpointer) sp);
  } else {
    disconnect_key_press_signal (sp);
    disconnect_button_press_signal (sp);
    disconnect_button_release_signal (sp);
    disconnect_motion_signal (sp);
  }
}


/*----------------------------------------------------------------------*/
/*                   Making the control panel                           */
/*----------------------------------------------------------------------*/

static gchar *display_lbl[] = {"Case label", "Point coords"};
void
cpanel_identify_make(ggobid *gg) {
  GtkWidget *btn;
  GtkWidget *opt;
  
  gg->control_panel[IDENT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER(gg->control_panel[IDENT]), 5);

  /*-- option menu --*/
  opt = gtk_option_menu_new ();
  gtk_widget_set_name (opt, "IDENTIFY:display_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Display either case label or coordinates of nearest point", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[IDENT]),
                      opt, false, false, 0);
  populate_option_menu (opt, display_lbl,
                        sizeof (display_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) display_cb, gg);

 /*-- button for removing all labels --*/
  btn = gtk_button_new_with_label ("Remove labels");
  gtk_widget_set_name (btn, "IDENTIFY:remove_sticky_labels");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
                        btn, "Remove all labels", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (id_remove_labels_cb),
                      gg);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[IDENT]),
                      btn, false, false, 1);

/*
 * button for making all labels sticky
*/
  btn = gtk_button_new_with_label ("Make all sticky");
  gtk_tooltips_set_tip (GTK_TOOLTIPS(gg->tips), btn,
    "Make all labels sticky, or persistent (to make the nearest point label sticky, click middle or right in the plot)",
    NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (id_all_sticky_cb),
                      (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[IDENT]),
                      btn, false, false, 1);

  gtk_widget_show_all (gg->control_panel[IDENT]);
}


/*--------------------------------------------------------------------*/
/*                      Control panel section                         */
/*--------------------------------------------------------------------*/

void
cpanel_identify_init (cpaneld *cpanel, ggobid *gg)
{
  cpanel->identify_display_type = ID_CASE_LABEL;
}

void
cpanel_identify_set (cpaneld *cpanel, ggobid *gg)
{
  GtkWidget *w;

  w = widget_find_by_name (gg->control_panel[IDENT],
                           "IDENTIFY:display_option_menu");
  gtk_option_menu_set_history (GTK_OPTION_MENU(w),
                               cpanel->identify_display_type);
}

