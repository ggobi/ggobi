/* jitter_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static GtkWidget *window = NULL;

void
jitter_vars_init () {
  xg.jitter_type = UNIFORM;
  xg.jitter_vgroup = true;
  xg.jitter_convex = true;
}

static void
jitter_cb (GtkButton *button)
{
  rejitter ();
}

/*
 * Set the degree of jittering
*/
static void
degree_cb (GtkAdjustment *adj, gpointer cbd) {
  if (xg.current_display->missing_p) {
    missing_jitter_value_set (adj->value);
    missing_rejitter ();
  } else {
    jitter_value_set (adj->value);
    rejitter ();
  }
}

static void
hide_cb (GtkWidget *w ) {
  gtk_widget_hide (w);
}

static gchar *type_lbl[] = {"Uniform", "Normal"};
static void type_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  xg.jitter_type = indx;
  rejitter ();
}

static void
vgroups_cb (GtkToggleButton *button)
{
  xg.jitter_vgroup = button->active;
}

void
jitter_popup_open () {

  GtkWidget *btn, *tgl, *lbl;
  GtkWidget *vbox, *vb;
  GtkWidget *sbar, *opt;
  GtkObject *adj;

  if (xg.nrows == 0)  /*-- if used before we have data --*/
    return;

  if (window == NULL) {

    jitter_vars_init ();
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "jitter data");
    
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (window), vbox);

/*
 * vbox for label and rangewidget
*/
    vb = gtk_vbox_new (true, 2);
    gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 1);

    lbl = gtk_label_new ("Degree of jitter:");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

    adj = gtk_adjustment_new (0.0, 0.0, 0.7, 0.01, .01, 0.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (degree_cb), NULL);

    sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), sbar,
      "Set the degree of jitter", NULL);
    gtk_scale_set_draw_value (GTK_SCALE (sbar), false);
    gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
    gtk_scale_set_digits (GTK_SCALE (sbar), 2);
    gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
    gtk_box_pack_start (GTK_BOX (vb), sbar, false, false, 0);

/*
 * Rejitter button
*/
    btn = gtk_button_new_with_label ("Jitter");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
      "Rejitter the data", NULL);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                       GTK_SIGNAL_FUNC (jitter_cb), (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);

/*
 * option menu
*/
    opt = gtk_option_menu_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
      "The jittering is either distributed uniform or normal", NULL);
    gtk_box_pack_start (GTK_BOX (vbox),
                        opt, false, false, 0);
    populate_option_menu (opt, type_lbl,
                          sizeof (type_lbl) / sizeof (gchar *),
                          type_cb);

/*
 * Jitter vgroups toggle
*/
    tgl = gtk_check_button_new_with_label ("Jitter vgroup");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), tgl,
      "Jitter each variable in the variable groups of this plot's selected variables",
      NULL);
    gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                       GTK_SIGNAL_FUNC (vgroups_cb), (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vbox), tgl,
      false, false, 3);
/*
 * Close button
*/
    btn = gtk_button_new_with_label ("Close");
    gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
                   GTK_SIGNAL_FUNC (hide_cb), (gpointer) window);
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, true, 2);
    gtk_widget_show_all (window);
  }

  gdk_window_raise (window->window);
}
