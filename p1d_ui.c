
#include <strings.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static GtkWidget *type_opt;
static gchar *type_lbl[] = {"Texturing", "ASH"};
static void type_cb (GtkWidget *w, gpointer cbd)
{
  cpaneld *cpanel = &current_display->cpanel;
  cpanel->p1d_type = GPOINTER_TO_INT (cbd);

  display_tailpipe (current_display);
}

static GtkObject *ash_smoothness_adj;
static void ash_smoothness_cb (GtkAdjustment *adj, gpointer cbd) {
  cpaneld *cpanel = &current_display->cpanel;

  /*-- adj->value ranges from .01 to .5 --*/
  cpanel->nASHes = (gint) ((gfloat) cpanel->nbins * (adj->value / 2.0));

  display_tailpipe (current_display);
}

static GtkObject *cycle_speed_adj;
static gboolean cycle_p = false;
static void cycle_cb (GtkToggleButton *button)
{
  cycle_p = button->active;
}
static void cycle_speed_cb (GtkAdjustment *adj, gpointer cbd) {
  g_printerr ("%d\n", ((gint) adj->value));
}

static gint direction = FORWARD;
static void chdir_cb (GtkButton *button)
{
  direction = -1 * direction;
}

static void scale_set_default_values (GtkScale *scale)
{
  gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_draw_value (scale, false);
}


void
cpanel_p1dplot_make () {
  GtkWidget *tgl, *btn, *vb;
  GtkWidget *sbar;
  
  control_panel[P1PLOT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (control_panel[P1PLOT]), 5);

/*
 * option menu
*/
  type_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), type_opt,
    "Display either textured dot plots or average shifted histograms", NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[P1PLOT]),
                      type_opt, false, false, 0);
  populate_option_menu (type_opt, type_lbl,
                        sizeof (type_lbl) / sizeof (gchar *),
                        type_cb);
/*
 * ASH smoothness
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (control_panel[P1PLOT]), vb,
    false, false, 0);

  gtk_box_pack_start (GTK_BOX (vb), gtk_label_new ("ASH smoothness:"),
    false, false, 0);

  /*-- value, lower, upper, step --*/
  ash_smoothness_adj = gtk_adjustment_new (0.19, 0.01, 0.5, 0.01, .01, 0.0);
  gtk_signal_connect (GTK_OBJECT (ash_smoothness_adj), "value_changed",
                      GTK_SIGNAL_FUNC (ash_smoothness_cb), NULL);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (ash_smoothness_adj));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), sbar,
    "Adjust ASH smoothness", NULL);
  gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
  gtk_scale_set_digits (GTK_SCALE (sbar), 2);

  gtk_box_pack_start (GTK_BOX (vb), sbar,
    false, false, 1);
/*
 * Cycling controls
*/

  tgl = gtk_check_button_new_with_label ("Cycle");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), tgl,
                        "Cycle through 1D plots", NULL);
  gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                      GTK_SIGNAL_FUNC (cycle_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[P1PLOT]),
                      tgl, false, false, 1);

  /* value, lower, upper, step_increment, page_increment, page_size */
  /* Note that the page_size value only makes a difference for
   * scrollbar widgets, and the highest value you'll get is actually
   * (upper - page_size). */
  cycle_speed_adj = gtk_adjustment_new (1.0, 0.0, 100.0, 1.0, 1.0, 0.0);
  gtk_signal_connect (GTK_OBJECT (cycle_speed_adj), "value_changed",
                      GTK_SIGNAL_FUNC (cycle_speed_cb), NULL);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (cycle_speed_adj));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), sbar,
    "Adjust cycling speed", NULL);
  scale_set_default_values (GTK_SCALE (sbar));

  gtk_box_pack_start (GTK_BOX (control_panel[P1PLOT]), sbar,
    false, false, 1);

  btn = gtk_button_new_with_label ("Change direction");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
    "Change cycling direction", NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[P1PLOT]),
                      btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (chdir_cb), NULL);

  gtk_widget_show_all (control_panel[P1PLOT]);
}


/*-- for all plot modes, for now: it excludes the changing variable --*/
void
cpanel_p1d_init (cpaneld *cpanel) {
  cpanel->nASHes = 20;
  cpanel->nbins = 200;
}

/*-- scatterplot only; need a different routine for parcoords --*/
void
cpanel_p1d_set (cpaneld *cpanel)
/*
 * To handle the case where there are multiple scatterplots
 * which may have different p1d options and parameters selected
*/
{
  /*-- Texturing or ASH --*/
  gtk_option_menu_set_history (GTK_OPTION_MENU (type_opt),
                               cpanel->p1d_type);

  /*-- ASH smoothness parameter --*/
  gtk_adjustment_set_value (GTK_ADJUSTMENT (ash_smoothness_adj),
    2 * (gfloat) cpanel->nASHes / (gfloat) cpanel->nbins);

  /*-- Cycling on or off --*/
  /*-- Cycling speed --*/
  /*-- Cycling direction --*/
}

