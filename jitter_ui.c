/* jitter_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

void
jitter_vars_init (datad *d, ggobid *gg) {
  d->jitter.type = UNIFORM;
  d->jitter.vgroup = true;
  d->jitter.convex = true;
}

static void
jitter_cb (GtkButton *button, ggobid *gg)
{
  datad *d = gg->current_display->d;

  /* 
   * if datad has changed, refuse to do anything until the
   * user has closed and reopened jitter_ui.window.
  */
  if (gg->jitter_ui.d != d) {
    g_printerr ("Close and reopen this window, please\n");
    return;
  }

  rejitter (d, gg);
}

/*
 * Set the degree of jittering
*/
static void
degree_cb (GtkAdjustment *adj, ggobid *gg) {
  datad *d = gg->current_display->d;

  /* 
   * if datad has changed, refuse to do anything until the
   * user has closed and reopened jitter_ui.window.
  */
  if (gg->jitter_ui.d != d) {
    g_printerr ("Close and reopen this window, please\n");
    return;
  }

  if (gg->current_display->missing_p) {
    missing_jitter_value_set (adj->value, d, gg);
    missing_rejitter (d, gg);
  } else {
    jitter_value_set (adj->value, d, gg);
    rejitter (d, gg);
  }
}

/*-- called when closed from the button --*/
static void
close_cb (GtkWidget *w ) {
  gtk_widget_hide (w);
}
/*-- called when closed from the window manager --*/
static void delete_cb (GtkWidget *w, GdkEvent *event, ggobid *gg) {
  gtk_widget_hide (gg->jitter_ui.window);
}

static gchar *type_lbl[] = {"Uniform", "Normal"};

static void type_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  ggobid *gg = GGobiFromWidget(w, true);
  datad *d = gg->current_display->d;

  /* 
   * if datad has changed, refuse to do anything until the
   * user has closed and reopened jitter_ui.window.
  */
  if (gg->jitter_ui.d != d) {
    g_printerr ("Close and reopen this window, please\n");
    return;
  }

  d->jitter.type = indx;
  rejitter (d, gg);
}

static void
vgroups_cb (GtkToggleButton *button, ggobid *gg)
{
  datad *d = gg->current_display->d;

  /* 
   * if datad has changed, refuse to do anything until the
   * user has closed and reopened jitter_ui.window.
  */
  if (gg->jitter_ui.d != d) {
    g_printerr ("Close and reopen this window, please\n");
    return;
  }

  d->jitter.vgroup = button->active;
}

void
jitter_window_open (ggobid *gg) {

  GtkWidget *btn, *tgl, *lbl;
  GtkWidget *vbox, *vb;
  GtkWidget *sbar, *opt;
  GtkObject *adj;
  datad *d = gg->current_display->d;

  if (d == NULL)  /*-- if used when we have no data --*/
    return;

  else {

    gg->jitter_ui.d = d;

    /*
     * if this particular datad object hasn't been active,
     * initialize it.
    */
    if (d->jitter.jitfacv == NULL) {
      jitter_vars_init (d, gg);
    }

    if (gg->jitter_ui.window == NULL) {

      gg->jitter_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_signal_connect (GTK_OBJECT (gg->jitter_ui.window), "delete_event",
                          GTK_SIGNAL_FUNC (delete_cb), (gpointer) gg);
      gtk_window_set_title (GTK_WINDOW (gg->jitter_ui.window), "jitter data");
    
      gtk_container_set_border_width (GTK_CONTAINER (gg->jitter_ui.window), 10);

      vbox = gtk_vbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (gg->jitter_ui.window), vbox);

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

      /*-- option menu --*/
      opt = gtk_option_menu_new ();
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
        "The jittering is either distributed uniform or normal", NULL);
      gtk_box_pack_start (GTK_BOX (vbox), opt, false, false, 0);
      populate_option_menu (opt, type_lbl,
                            sizeof (type_lbl) / sizeof (gchar *),
                            type_cb, gg);

      /*-- Jitter vgroups toggle --*/
      tgl = gtk_check_button_new_with_label ("Jitter vgroup");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
        "Jitter each variable in the variable groups of this plot's selected variables",
        NULL);
      gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                         GTK_SIGNAL_FUNC (vgroups_cb), (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (vbox), tgl,
        false, false, 3);

      /*-- Close button --*/
      btn = gtk_button_new_with_label ("Close");
      gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
                                 GTK_SIGNAL_FUNC (close_cb),
                                 (gpointer) gg->jitter_ui.window);
      gtk_box_pack_start (GTK_BOX (vbox), btn, false, true, 2);
    }

    gtk_widget_show_all (gg->jitter_ui.window);
  }

  gdk_window_raise (gg->jitter_ui.window->window);
}
