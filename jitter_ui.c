/* jitter_ui.c */
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
#include "vars.h"
#include "externs.h"

void
jitter_vars_init (datad *d, ggobid *gg) {
  d->jitter.type = UNIFORM;
  d->jitter.convex = true;
}

static void
jitter_cb (GtkButton *w, ggobid *gg)
{
  GtkWidget *clist = get_clist_from_object (GTK_OBJECT(gg->jitter_ui.window));
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gint *vars = (gint *) g_malloc (d->ncols * sizeof(gint));
  gint nvars = get_selections_from_clist (d->ncols, vars, clist, d);

  if (nvars) {
    rejitter (vars, nvars, d, gg);
    g_free (vars);
  }
}

/*
 * Set the degree of jittering
*/
static void
degree_cb (GtkAdjustment *adj, ggobid *gg) {
  GtkWidget *clist = get_clist_from_object (GTK_OBJECT(gg->jitter_ui.window));
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gint *vars = (gint *) g_malloc (d->ncols * sizeof(gint));
  gint nvars = get_selections_from_clist (d->ncols, vars, clist, d);

  jitter_value_set (adj->value, d, gg);
  if (nvars) {
    rejitter (vars, nvars, d, gg);
    g_free (vars);
  }
}

/*-- called when closed from the close button --*/
static void close_btn_cb (GtkWidget *w, ggobid *gg) {
  gtk_widget_hide (gg->jitter_ui.window);
}
/*-- called when closed from the window manager --*/
static void
close_wmgr_cb (GtkWidget *w, GdkEvent *event, ggobid *gg) {
  gtk_widget_hide (gg->jitter_ui.window);
}

static gchar *type_lbl[] = {"Uniform", "Normal"};

static void type_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  ggobid *gg = GGobiFromWidget(w, true);
  GtkWidget *clist = get_clist_from_object (GTK_OBJECT(gg->jitter_ui.window));
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gint *vars = (gint *) g_malloc (d->ncols * sizeof(gint));
  gint nvars = get_selections_from_clist (d->ncols, vars, clist, d);

  d->jitter.type = indx;

  if (nvars) {
     rejitter (vars, nvars, d, gg);
     g_free (vars);
  }
}

void
jitter_window_open (ggobid *gg) {

  GtkWidget *btn, *lbl;
  GtkWidget *vbox, *vb, *hb;
  GtkWidget *sbar, *opt;
  GtkObject *adj;
  GtkWidget *notebook;

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length (gg->d) == 0) 
    return;

  else {

    if (gg->jitter_ui.window == NULL) {

      gg->jitter_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

      /*-- suggested by Gordon Deane --*/
      gtk_window_set_default_size(GTK_WINDOW(gg->jitter_ui.window), 200, 400);
      /*-- 400 looks too big on the laptop, trying other numbers   --*/
      gtk_window_set_default_size(GTK_WINDOW(gg->jitter_ui.window), 200, 250);
      /*--                           --*/

      gtk_signal_connect (GTK_OBJECT (gg->jitter_ui.window), "delete_event",
                          GTK_SIGNAL_FUNC (close_wmgr_cb), (gpointer) gg);
      gtk_window_set_title (GTK_WINDOW (gg->jitter_ui.window), "jitter data");
    
      gtk_container_set_border_width (GTK_CONTAINER (gg->jitter_ui.window), 10);

      vbox = gtk_vbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (gg->jitter_ui.window), vbox);

      /* Create a notebook, set the position of the tabs */
      notebook = create_variable_notebook (vbox,
        GTK_SELECTION_EXTENDED, all_vartypes, all_datatypes,
        (GtkSignalFunc) NULL, gg);

      /*-- option menu --*/
      opt = gtk_option_menu_new ();
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
        "The jittering is either distributed uniform or normal", NULL);
      gtk_box_pack_start (GTK_BOX (vbox), opt, false, false, 0);
      populate_option_menu (opt, type_lbl,
        sizeof (type_lbl) / sizeof (gchar *),
        (GtkSignalFunc) type_cb, "GGobi", gg);

      /*-- vbox for label and rangewidget --*/
      vb = gtk_vbox_new (true, 2);
      gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 1);

      lbl = gtk_label_new ("Degree of jitter:");
      gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

      adj = gtk_adjustment_new (0.0, 0.0, 0.7, 0.01, .01, 0.0);
      gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                          GTK_SIGNAL_FUNC (degree_cb), (gpointer) gg);

      sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
        "Set the degree of jitter", NULL);
      gtk_scale_set_draw_value (GTK_SCALE (sbar), false);
      gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
      gtk_scale_set_digits (GTK_SCALE (sbar), 2);
      gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
      gtk_box_pack_start (GTK_BOX (vb), sbar, false, false, 0);

      /*-- Rejitter button --*/
      btn = gtk_button_new_with_label ("Jitter");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
        "Rejitter the data", NULL);
      gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                         GTK_SIGNAL_FUNC (jitter_cb), (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);

      /*-- Close button --*/
      gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new(), false, true, 2);
      hb = gtk_hbox_new (false, 2);
      gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 1);

      btn = gtk_button_new_with_label ("Close");
      gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                          GTK_SIGNAL_FUNC (close_btn_cb), gg);
      gtk_box_pack_start (GTK_BOX (hb), btn, true, false, 0);
    }

    gtk_object_set_data (GTK_OBJECT (gg->jitter_ui.window),
      "notebook", notebook);
    gtk_widget_show_all (gg->jitter_ui.window);
  }

  gdk_window_raise (gg->jitter_ui.window->window);
}
