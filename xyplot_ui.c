/*-- xyplot_ui.c --*/

#include <strings.h>
#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

/* Move to cpanel.h */


static const gchar *const fix_axis_lbl[] = {"No fixed axes", "Fix X", "Fix Y"};
static void fix_axis_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", fix_axis_lbl[indx]);
}

static void cycle_cb (GtkToggleButton *button, ggobid *gg)
{
  gg->app.cycle_p = button->active;
}
static void scale_set_default_values (GtkScale *scale )
{
  gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_draw_value (scale, false);
}

static void cycle_speed_cb (GtkAdjustment *adj, gpointer cbd) {
  g_printerr ("%d\n", ((gint) adj->value));
}

static void chdir_cb (GtkButton *button, ggobid* gg)
{
  gg->app.direction = -1 * gg->app.direction;
}

void
cpanel_xyplot_make (ggobid *gg) {
  GtkWidget *cycle_tgl, *chdir_btn, *cycle_sbar, *opt;
  GtkObject *adj;
  
  gg->control_panel[XYPLOT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[XYPLOT]), 5);

  cycle_tgl = gtk_check_button_new_with_label ("Cycle");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), cycle_tgl,
    "Cycle through pairwise plots", NULL);
  gtk_signal_connect (GTK_OBJECT (cycle_tgl), "toggled",
                     GTK_SIGNAL_FUNC (cycle_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[XYPLOT]), cycle_tgl,
    false, false, 3);

/*
 * make an option menu
*/
  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Fix one of the axes during plot cycling or let them both float", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[XYPLOT]), opt,
    false, false, 0);
  populate_option_menu (opt, (gchar**) fix_axis_lbl,
                        sizeof (fix_axis_lbl) / sizeof (gchar *),
                        fix_axis_cb, gg);
  
  /* value, lower, upper, step_increment, page_increment, page_size */
  /* Note that the page_size value only makes a difference for
   * scrollbar widgets, and the highest value you'll get is actually
   * (upper - page_size). */
  adj = gtk_adjustment_new (1.0, 0.0, 100.0, 1.0, 1.0, 0.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                      GTK_SIGNAL_FUNC (cycle_speed_cb), NULL);

  cycle_sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  scale_set_default_values (GTK_SCALE (cycle_sbar));

  gtk_box_pack_start (GTK_BOX (gg->control_panel[XYPLOT]),
                      cycle_sbar, false, false, 1);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), cycle_sbar,
    "Adjust cycling speed", NULL);

  chdir_btn = gtk_button_new_with_label ("Change direction");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), chdir_btn,
    "Change cycling direction", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[XYPLOT]),
                      chdir_btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (chdir_btn), "clicked",
                      GTK_SIGNAL_FUNC (chdir_cb), gg);

  gtk_widget_show_all (gg->control_panel[XYPLOT]);
}
