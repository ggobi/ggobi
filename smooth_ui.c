/* smooth_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static GtkWidget *window = NULL;
static gchar *smoother_lbl[] = {"Mean", "Median", "Nadaraya-Watson", "Spline" };

static void
smoother_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", smoother_lbl[indx]);
}

static void
smooth_cb (GtkToggleButton *button)
{
  g_printerr ("active: %d\n", button->active);
}

static void
groups_cb (GtkToggleButton *button)
{
  g_printerr ("use groups: %d\n", button->active);
}

static void
window_cb (GtkToggleButton *button)
{
  g_printerr ("show window: %d\n", button->active);
}

void
width_cb (GtkAdjustment *adj, gpointer cbd) {
  g_printerr ("width %f\n", adj->value);
}

static void
hide_cb (GtkWidget *w ) {
  gtk_widget_hide (w);
}

void
open_smooth_popup () {

  GtkWidget *tgl, *lbl;
  GtkWidget *vbox, *vb;
  GtkWidget *sbar, *opt;
  GtkObject *adj;
  extern void populate_option_menu (GtkWidget *, gchar **, gint,
                                    GtkSignalFunc);

  if (window == NULL) {
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "smooth data");
    
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (window), vbox);

/*
 * Smooth toggle
*/
    tgl = gtk_check_button_new_with_label ("Smooth");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), tgl,
      "Add one or more smoothed lines to the current plot", NULL);
    gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                       GTK_SIGNAL_FUNC (smooth_cb), (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vbox), tgl, false, false, 3);

/*
 * smoothers option menu 
*/
    vb = gtk_vbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 1);

    lbl = gtk_label_new ("Smoothing functions:"),
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vbox), lbl, false, false, 0);

    opt = gtk_option_menu_new ();
    gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
      "Set the smoothing function", NULL);
    gtk_box_pack_start (GTK_BOX (vbox), opt, false, false, 0);
    populate_option_menu (opt, smoother_lbl,
                          sizeof (smoother_lbl) / sizeof (gchar *),
                          smoother_cb);

/*
 * vbox for label and rangewidget
*/
    vb = gtk_vbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 1);

    lbl = gtk_label_new ("Width:");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

    adj = gtk_adjustment_new (1.0, 0.0, 1.0, 0.01, .01, 0.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (width_cb), NULL);

    sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), sbar,
      "Set the width of the smoothing window", NULL);
    gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
    gtk_scale_set_digits (GTK_SCALE (sbar), 2);
    gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
    gtk_box_pack_start (GTK_BOX (vb), sbar, false, false, 0);

/*
 * Use color groups toggle
*/
    tgl = gtk_check_button_new_with_label ("Use groups");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), tgl,
      "Add one smoothed line for each point color", NULL);
    gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                       GTK_SIGNAL_FUNC (groups_cb), (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vbox), tgl,
      false, false, 3);

/*
 * Show smoothing window
*/
    tgl = gtk_check_button_new_with_label ("Show window");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), tgl,
      "Show the smoothing window on the scatterplot display", NULL);
    gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                       GTK_SIGNAL_FUNC (window_cb), (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vbox), tgl,
      false, false, 3);

/*
    gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                   GTK_SIGNAL_FUNC (hide_cb), (gpointer) window);
*/

    gtk_widget_show_all (window);
  }

  gdk_window_raise (window->window);
}
