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
  cpanel->id_display_type = GPOINTER_TO_INT (cbd);
  displays_plot (NULL, QUICK, gg);
}

static void identify_target_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->id_target_type = (enum idtargetd) GPOINTER_TO_INT (cbd);
  displays_plot (NULL, QUICK, gg);
}

static void
recenter_cb (GtkWidget *w, ggobid *gg)
{
  datad *d = gg->current_display->d;
  gint k = -1;
  if (g_slist_length (d->sticky_ids) >= 1) {
    k = GPOINTER_TO_INT (d->sticky_ids->data);
  }
  recenter_data (k, d, gg);
}

static void
id_remove_labels_cb (GtkWidget *w, ggobid *gg)
{
  displayd *dsp = gg->current_display;
  cpaneld *cpanel = &gg->current_display->cpanel;
  datad *d;
  gboolean ok = true;

  if (cpanel->id_target_type == identify_points)
    d = dsp->d;
  else {
    d = dsp->e;
    ok = (d != NULL);
  }

  g_slist_free (d->sticky_ids);
  d->sticky_ids = (GSList *) NULL;

  /* This will become an event on the datad when we move to
     Gtk objects (soon now!) */
  gtk_signal_emit(GTK_OBJECT(gg), GGobiSignals[STICKY_POINT_REMOVED_SIGNAL], 
    -1, (gint) UNSTICKY, d);

  displays_plot (NULL, QUICK, gg);
}

static void
id_all_sticky_cb (GtkWidget *w, ggobid *gg)
{
  gint i, m;
  datad *d;
  displayd *dsp = gg->current_display;
  cpaneld *cpanel = &dsp->cpanel;

  if (cpanel->id_target_type == identify_edges) {
    if (dsp->e != NULL) d = dsp->e;
  } else d = dsp->d;
  
  /*-- clear the list before adding to avoid redundant entries --*/
  g_slist_free (d->sticky_ids);
  d->sticky_ids = (GSList *) NULL;

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot.els[m];
    d->sticky_ids = g_slist_append (d->sticky_ids, GINT_TO_POINTER (i));
  }

  /* This will become an event on the datad when we move to
     Gtk objects (soon now!) */
  gtk_signal_emit(GTK_OBJECT(gg),
    GGobiSignals[STICKY_POINT_ADDED_SIGNAL], -1,
    (gint) STICKY, d);
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


static gint
motion_notify_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp)
{
  gint k;
  ggobid *gg = GGobiFromSPlot(sp);
  datad *d = gg->current_display->d;
  gboolean button1_p, button2_p;
  gint nd = g_slist_length (gg->d);
  cpaneld *cpanel = &gg->current_display->cpanel;
  GGobiPointMoveEvent ev;

/*
 * w = sp->da
 * sp = gtk_object_get_data (GTK_OBJECT (w), "splotd"));
*/

  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);
 
  if (GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
    gboolean changed;
    gboolean (*f)(icoords, splotd *sp, datad *, ggobid *);

    f = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT_GET_CLASS(sp))->identify_notify;
    if(f) {
      changed = f(sp->mousepos, sp, d, gg);
      if (changed) {
        displays_plot (NULL, QUICK, gg);
      }
      return(true);
    }
  }

  if (cpanel->id_target_type == identify_points) {
    k = find_nearest_point (&sp->mousepos, sp, d, gg);
    d->nearest_point = k;

    /*-- link by id --*/
    if (nd > 1) identify_link_by_id (k, d, gg);
    /*-- --*/

    if (k != d->nearest_point_prev) {
      displays_plot (NULL, QUICK, gg);

#ifdef EXPLICIT_IDENTIFY_HANDLER 
      if (gg->identify_handler.handler) {
        (gg->identify_handler.handler)(gg->identify_handler.user_data,
          k, sp, w, gg);
      }
#endif

      if (k != d->nearest_point_prev) {
        ev.d = d;
        ev.id = k;
        /* This will become an event on the datad when we move to
           Gtk objects (soon now!) */
        gtk_signal_emit(GTK_OBJECT(gg), GGobiSignals[IDENTIFY_POINT_SIGNAL],
          sp, k, d); 

        displays_plot (NULL, QUICK, gg);
        d->nearest_point_prev = k;
      }
    }
  } else {
    datad *e = gg->current_display->e;
    if (e && e->edge.n) {
      k = find_nearest_edge (sp, gg->current_display, gg);
      e->nearest_point = k;
      if (e->nearest_point != e->nearest_point_prev) {
        ev.d = e;
        ev.id = k;
        /*-- perhaps this should be an IDENTIFY_EDGE_SIGNAL ... --*/
        gtk_signal_emit(GTK_OBJECT(gg), GGobiSignals[IDENTIFY_POINT_SIGNAL],
          sp, k, e); 

        displays_plot (NULL, QUICK, gg);
        e->nearest_point_prev = k;
      }
    }
  }

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
  displayd *dsp = sp->displayptr;
  cpaneld *cpanel = &dsp->cpanel;
  datad *d;

  if (cpanel->id_target_type == identify_edges) {
    if (dsp->e != NULL) d = dsp->e;
  } else d = dsp->d;

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


/*----------------------------------------------------------------------*/
/*                   Making the control panel                           */
/*----------------------------------------------------------------------*/

static gchar *display_lbl[] = {
  "Record label",
  "Record number",
  "Variable labels",
  "Record id",
  };
static gchar *target_lbl[] = {
  "Points",
  "Edges",
  };
void
cpanel_identify_make(ggobid *gg) {
  GtkWidget *btn, *opt;
  GtkWidget *notebook;
  GtkWidget *frame, *framevb;
  
  gg->control_panel[IDENT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER(gg->control_panel[IDENT]), 5);

  /*-- provide a variable list so that any variable can be the label --*/
  notebook = create_variable_notebook (gg->control_panel[IDENT],
    GTK_SELECTION_EXTENDED, all_vartypes, all_datatypes,
    (GtkSignalFunc) NULL, gg);
  gtk_object_set_data (GTK_OBJECT (gg->control_panel[IDENT]),
    "notebook", notebook);

  /*-- option menu --*/
  opt = gtk_option_menu_new ();
  gtk_widget_set_name (opt, "IDENTIFY:display_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "How to construct the label to be displayed: the record label, record number, a label constructed using variables selected in the list above, or the record id",
    NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[IDENT]),
                      opt, false, false, 0);
  populate_option_menu (opt, display_lbl,
    sizeof (display_lbl) / sizeof (gchar *),
    (GtkSignalFunc) display_cb, "GGobi", gg);

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

  /*-- option menu --*/
  opt = gtk_option_menu_new ();
  gtk_widget_set_name (opt, "IDENTIFY:target_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Label points or edges",
    NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[IDENT]),
    opt, false, false, 0);
  populate_option_menu (opt, target_lbl,
    sizeof (target_lbl) / sizeof (gchar *),
    (GtkSignalFunc) identify_target_cb, "GGobi", gg);


  /*-- frame around button for resetting center --*/
  frame = gtk_frame_new ("Recenter data");
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[IDENT]), frame,
    false, false, 3);

  framevb = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (framevb), 4);
  gtk_container_add (GTK_CONTAINER (frame), framevb);

  btn = gtk_button_new_with_label ("Recenter");
  gtk_widget_set_name (btn, "IDENT:recenter_btn");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Make one point sticky, and then click here to recenter the data around that point. (If there are no sticky labels, restore default centering.)",
    NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (recenter_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (framevb), btn,
    false, false, 0);
  /*-- --*/

  gtk_widget_show_all (gg->control_panel[IDENT]);
}


/*--------------------------------------------------------------------*/
/*                      Control panel section                         */
/*--------------------------------------------------------------------*/

void
cpanel_identify_init (cpaneld *cpanel, ggobid *gg)
{
  cpanel->id_display_type = ID_RECORD_LABEL;
}

void
cpanel_identify_set (cpaneld *cpanel, ggobid *gg)
{
  GtkWidget *w;

  w = widget_find_by_name (gg->control_panel[IDENT],
                           "IDENTIFY:display_option_menu");
  gtk_option_menu_set_history (GTK_OPTION_MENU(w),
                               cpanel->id_display_type);

  w = widget_find_by_name (gg->control_panel[IDENT],
                           "IDENTIFY:target_option_menu");
  gtk_option_menu_set_history (GTK_OPTION_MENU(w),
                               (gint) cpanel->id_target_type);
}

