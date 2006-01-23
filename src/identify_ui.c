/*-- identify_ui.c --*/
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

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include "vars.h"
#include "externs.h"

/*static void display_cb (GtkWidget *w, ggobid *gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->id_display_type = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
  displays_plot (NULL, QUICK, gg);
}*/

static void identify_target_cb (GtkWidget *w, ggobid *gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->id_target_type = (enum idtargetd) gtk_combo_box_get_active(GTK_COMBO_BOX(w));
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
  g_signal_emit(G_OBJECT(gg), GGobiSignals[STICKY_POINT_REMOVED_SIGNAL], 0,
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
  g_signal_emit(G_OBJECT(gg),
    GGobiSignals[STICKY_POINT_ADDED_SIGNAL], 0, -1,
    (gint) STICKY, d);
  displays_plot (NULL, QUICK, gg);
}

enum { RECORD_ID_INDEX = -3, RECORD_LABEL_INDEX, RECORD_NUMBER_INDEX };

static
void
label_selected_cb(GtkTreeSelection *treesel, ggobid *gg)
{
	GtkTreeModel *model;
	gint *vars, nvars, i;
	cpaneld *cpanel = &gg->current_display->cpanel;
	vars = get_selections_from_tree_view(GTK_WIDGET(gtk_tree_selection_get_tree_view(treesel)), &nvars);
	cpanel->id_display_type = 0;
	for (i = 0; i < nvars; i++) {
		if (vars[i] < 0)
			cpanel->id_display_type |= 1 << -vars[i];
		else cpanel->id_display_type |= 1;
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

  return false;
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
 * sp = g_object_get_data(G_OBJECT (w), "splotd"));
*/

  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);
 
  if (GGOBI_IS_EXTENDED_SPLOT(sp)) {
    gboolean changed;
    gboolean (*f)(icoords, splotd *sp, datad *, ggobid *);

    f = GGOBI_EXTENDED_SPLOT_GET_CLASS(sp)->identify_notify;
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
           Gtk objects (soon now!) - note: this came long ago */
        g_signal_emit(G_OBJECT(gg), GGobiSignals[IDENTIFY_POINT_SIGNAL], 0,
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
        g_signal_emit(G_OBJECT(gg), GGobiSignals[IDENTIFY_POINT_SIGNAL], 0,
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
    if(GGOBI_IS_WINDOW_DISPLAY(display))
      sp->key_press_id = g_signal_connect (G_OBJECT (GGOBI_WINDOW_DISPLAY(display)->window),
        "key_press_event", G_CALLBACK(key_press_cb), (gpointer) sp);

    sp->press_id = g_signal_connect (G_OBJECT (sp->da),
      "button_press_event", G_CALLBACK(button_press_cb), (gpointer) sp);
    sp->release_id = g_signal_connect (G_OBJECT (sp->da),
      "button_release_event", G_CALLBACK(button_release_cb), (gpointer) sp);
    sp->motion_id = g_signal_connect (G_OBJECT (sp->da),
      "motion_notify_event", G_CALLBACK(motion_notify_cb), (gpointer) sp);
  } else {
    disconnect_key_press_signal (sp);
    disconnect_button_press_signal (sp);
    disconnect_button_release_signal (sp);
    disconnect_motion_signal (sp);
  }
}

static const gchar *label_prefices[] = {
	"<i>Record Id</i>",
	"<i>Record Label</i>",
	"<i>Record Number</i>"
};

static const gchar **label_prefix_func(GtkWidget *notebook, datad *d, gint *sel_prefix, gint *n_prefices)
{
	gint offset = d->rowIds ? 0 : 1;
	*n_prefices = G_N_ELEMENTS(label_prefices) - offset;
	*sel_prefix = 1 - offset;
	return(label_prefices + offset);
}

/*----------------------------------------------------------------------*/
/*                   Making the control panel                           */
/*----------------------------------------------------------------------*/


/*	
static gchar *display_lbl[] = {
  "Record id",
  "Record label",
  "Record number",
  "Variable labels",
  };*/
static gchar *target_lbl[] = {
  "Points",
  "Edges",
  };
void
cpanel_identify_make(ggobid *gg) {
  modepaneld *panel;
  GtkWidget *btn, *opt;
  GtkWidget *notebook;
  GtkWidget *frame, *framevb;
  
  panel = (modepaneld *) g_malloc(sizeof(modepaneld));
  gg->control_panels = g_list_append(gg->control_panels, (gpointer) panel);
  panel->name = g_strdup(GGOBI(getIModeName)(IDENT));
  panel->w = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER(panel->w), 5);

  /*-- provide a variable list so that any variable can be the label --*/
  /*opt = gtk_combo_box_new_text ();*/ /* create combo box before notebook */
  notebook = create_prefixed_variable_notebook (panel->w, GTK_SELECTION_MULTIPLE, 
  	all_vartypes, all_datatypes, G_CALLBACK(label_selected_cb), NULL, gg, label_prefix_func);
  g_object_set_data(G_OBJECT (panel->w),
    "notebook", notebook);
  /*gtk_combo_box_set_active(GTK_COMBO_BOX(opt), ID_RECORD_LABEL);*/

  /*-- option menu --*/
  /* created above */
  /*gtk_widget_set_name (opt, "IDENTIFY:display_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "How to construct the label to be displayed: the record label, record number, a label constructed using variables selected in the list above, or the record id",
    NULL);
  gtk_box_pack_start (GTK_BOX (panel->w),
                      opt, false, false, 0);
  populate_combo_box (opt, display_lbl, G_N_ELEMENTS(display_lbl),
    G_CALLBACK(display_cb), gg);
*/
 /*-- button for removing all labels --*/
  btn = gtk_button_new_with_mnemonic ("_Remove labels");
  gtk_widget_set_name (btn, "IDENTIFY:remove_sticky_labels");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
                        btn, "Remove all labels", NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (id_remove_labels_cb),
                      gg);
  gtk_box_pack_start (GTK_BOX (panel->w),
                      btn, false, false, 1);

/*
 * button for making all labels sticky
*/
  //btn = gtk_button_new_with_mnemonic ("Make all _sticky");
  btn = gtk_button_new_with_mnemonic ("Label all");
  gtk_tooltips_set_tip (GTK_TOOLTIPS(gg->tips), btn,
    "Make all labels sticky, or persistent (to make the nearest point label sticky, click middle or right in the plot)",
    NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (id_all_sticky_cb),
                      (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (panel->w),
                      btn, false, false, 1);

  /*-- option menu --*/
  opt = gtk_combo_box_new_text ();
  gtk_widget_set_name (opt, "IDENTIFY:target_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Label points or edges",
    NULL);
  gtk_box_pack_start (GTK_BOX (panel->w),
    opt, false, false, 0);
  populate_combo_box (opt, target_lbl, G_N_ELEMENTS(target_lbl),
    G_CALLBACK(identify_target_cb), gg);


  /*-- frame around button for resetting center --*/
  frame = gtk_frame_new ("Recenter data");
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (panel->w), frame,
    false, false, 3);

  framevb = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (framevb), 4);
  gtk_container_add (GTK_CONTAINER (frame), framevb);

  btn = gtk_button_new_with_mnemonic ("Re_center");
  gtk_widget_set_name (btn, "IDENT:recenter_btn");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Make one point sticky, and then click here to recenter the data around that point. (If there are no sticky labels, restore default centering.)",
    NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (recenter_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (framevb), btn,
    false, false, 0);
  /*-- --*/

  gtk_widget_show_all (panel->w);
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
cpanel_identify_set (displayd *display, cpaneld *cpanel, ggobid *gg)
{
  GtkWidget *w;
  GtkWidget *pnl = mode_panel_get_by_name(GGOBI(getIModeName)(IDENT), gg);

  if (pnl == (GtkWidget *) NULL)
    return;

  /*w = widget_find_by_name (pnl, "IDENTIFY:display_option_menu");
  gtk_combo_box_set_active (GTK_COMBO_BOX(w),
                               cpanel->id_display_type);
*/
  w = widget_find_by_name (pnl, "IDENTIFY:target_option_menu");
  gtk_combo_box_set_active (GTK_COMBO_BOX(w),
                               (gint) cpanel->id_target_type);
}

