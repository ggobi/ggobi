/* transform_ui.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*
static void
hide_cb (GtkWidget *w ) {
  gtk_widget_hide (w);
}
*/

static gchar *stage0_lbl[] = {"No transformation",
                              "Raise minimum to 0",
                              "Raise minimum to 1",
                              "Negative"};
static void stage0_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  ggobid *gg = GGobiFromWidget(w, true);
  datad *d = gg->current_display->d;

  transform (0, indx, -99., d, gg);
}

static gchar *stage1_lbl[] = {"No transformation",
                              "Box-Cox",
                              "Log base 10",
                              "Inverse",
                              "Absolute value",
                              "Scale to [a,b]",
                              };
static void
stage1_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  gint indx = GPOINTER_TO_INT (cbd);
  datad *d = gg->current_display->d;

  transform (1, indx, gg->tform_ui.boxcox_adj->value, d, gg);
}

/*
 * Set the spin widget's adjustment->step_increment to adj->value
*/
void boxcox_cb (GtkAdjustment *adj, ggobid *gg)
{
  datad *d = gg->current_display->d;
  transform (1, BOXCOX, adj->value, d, gg);
}

gfloat
scale_get_a (ggobid *gg) {
  gchar *val_str;
  gfloat val = 0;  /*-- default value --*/

  val_str = gtk_editable_get_chars (GTK_EDITABLE (gg->tform_ui.entry_a), 0, -1);
  if (val_str != NULL && strlen (val_str) > 0) {
    val = (gfloat) atof (val_str);
    g_free (val_str);
  }

  return val;
}
gfloat
scale_get_b (ggobid *gg) {
  gchar *val_str;
  gfloat val = 1;  /*-- default value --*/

  val_str = gtk_editable_get_chars (GTK_EDITABLE (gg->tform_ui.entry_b), 0, -1);
  if (val_str != NULL && strlen (val_str) > 0) {
    val = (gfloat) atof (val_str);
    g_free (val_str);
  }

  return val;
}

static gchar *stage2_lbl[] = {"No transformation",
                              "Standardize",
                              "Sort",
                              "Rank",
                              "Normal score",
                              "Z-score",
                              "Discretize: 2 levels"
                              };
static void stage2_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  datad *d = gg->current_display->d;
  gint indx = GPOINTER_TO_INT (cbd);

  transform (2, indx, -99, d, gg);
}

static void tform_reset_cb (GtkWidget *w, ggobid *gg)
{
  gint j;
  datad *d = gg->current_display->d;

  for (j=0; j<d->ncols; j++) {
    transform0_values_set (NO_TFORM0, j, d, gg);
    transform1_values_set (NO_TFORM1, 1.0, j, d, gg);
    transform2_values_set (NO_TFORM2, j, d, gg);

    transform1_apply (j, d, gg);
    transform2_apply (j, d, gg);

    tform_label_update (j, d, gg);
  }

  limits_set (true, true, d, gg);  
  vartable_limits_set (d);
  vartable_stats_set (d);

  tform_to_world (d, gg);
  displays_tailpipe (REDISPLAY_PRESENT, gg);
}

void
transform_window_open (ggobid *gg) 
{

  GtkWidget *lbl, *hbox;
  GtkWidget *vbox, *frame, *hb, *vb, *btn;
  GtkWidget *spinner;
  datad *d = gg->current_display->d;

  if (d == NULL || d->nrows == 0)  /*-- if used before we have data --*/
    return;

  if (gg->tform_ui.window == NULL) {
    GtkStyle *style;
    gint lbearing, rbearing, width, ascent, descent;
    
    gg->tform_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->tform_ui.window),
                          "transform variables");
    
    gtk_container_set_border_width (GTK_CONTAINER (gg->tform_ui.window), 10);

/*
 * Divide the window:  functions on the left, labels on the right
*/
    hbox = gtk_hbox_new (false, 1);
    gtk_container_border_width (GTK_CONTAINER (hbox), 1);
    gtk_container_add (GTK_CONTAINER (gg->tform_ui.window), hbox);

/*
 * Transformations
*/
    vbox = gtk_vbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, false, false, 1);

    /*
     * Stage 0: Domain adjustment
    */
    frame = gtk_frame_new ("Stage 0");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame, true, false, 1);

    gg->tform_ui.stage0_opt = gtk_option_menu_new ();
    gtk_container_set_border_width (GTK_CONTAINER (gg->tform_ui.stage0_opt), 4);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->tform_ui.stage0_opt,
      "Stage 0: Adjust the domain of the variables",
      NULL);
    populate_option_menu (gg->tform_ui.stage0_opt, stage0_lbl,
                          sizeof (stage0_lbl) / sizeof (gchar *),
                          (GtkSignalFunc) stage0_cb, gg);
    gtk_container_add (GTK_CONTAINER (frame), gg->tform_ui.stage0_opt);

    /*
     * Stage 1: Power transformations et al
    */
    frame = gtk_frame_new ("Stage 1");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame, true, false, 1);

    vb = gtk_vbox_new (false, 5);
    gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
    gtk_container_add (GTK_CONTAINER (frame), vb);

    gg->tform_ui.stage1_opt = gtk_option_menu_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->tform_ui.stage1_opt,
      "Stage 1: Data-independent transformations, preserving user-defined limits",
      NULL);
    populate_option_menu (gg->tform_ui.stage1_opt, stage1_lbl,
                          sizeof (stage1_lbl) / sizeof (gchar *),
                          (GtkSignalFunc) stage1_cb, gg);
    gtk_box_pack_start (GTK_BOX (vb), gg->tform_ui.stage1_opt, true, false, 1);

    /*-- label and spin button for Box-Cox parameter --*/
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vb), hb, true, false, 2);
    
    lbl = gtk_label_new ("Box-Cox param:");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);
    gg->tform_ui.boxcox_adj = (GtkAdjustment *) gtk_adjustment_new (1.0,
                          -4, 5, 0.05, .5, 0.0);
    spinner = gtk_spin_button_new (gg->tform_ui.boxcox_adj, 0, 3);

    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinner,
      "Set the Box-Cox power function parameter", NULL);
    gtk_box_pack_end (GTK_BOX (hb), spinner, true, true, 0);
    gtk_signal_connect (GTK_OBJECT (gg->tform_ui.boxcox_adj), "value_changed",
		                GTK_SIGNAL_FUNC (boxcox_cb),
		                (gpointer) gg);

    /*-- labels and entries for scaling limits --*/
    style = gtk_widget_get_style (spinner);
    gdk_text_extents (style->font, 
      "999999999", strlen ("999999999"),
      &lbearing, &rbearing, &width, &ascent, &descent);

    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vb), hb, true, false, 2);

    lbl = gtk_label_new ("a:");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);

    gg->tform_ui.entry_a = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (gg->tform_ui.entry_a), "0");
    gtk_widget_set_usize (gg->tform_ui.entry_a, width, -1);
    gtk_box_pack_start (GTK_BOX (hb), gg->tform_ui.entry_a, false, false, 0);

    lbl = gtk_label_new ("b:");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);

    gg->tform_ui.entry_b = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (gg->tform_ui.entry_b), "1");
    gtk_widget_set_usize (gg->tform_ui.entry_b, width, -1);
    gtk_box_pack_start (GTK_BOX (hb), gg->tform_ui.entry_b, false, false, 0);

    /*
     * Stage 2: Another standardization step
    */
    frame = gtk_frame_new ("Stage 2");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame, true, false, 1);

    gg->tform_ui.stage2_opt = gtk_option_menu_new ();
    gtk_container_set_border_width (GTK_CONTAINER (gg->tform_ui.stage2_opt), 4);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->tform_ui.stage2_opt,
      "Stage 2: Data-dependent transformations, ignoring user-defined limits",
      NULL);
    populate_option_menu (gg->tform_ui.stage2_opt, stage2_lbl,
                          sizeof (stage2_lbl) / sizeof (gchar *),
                          (GtkSignalFunc) stage2_cb, gg);
    gtk_container_add (GTK_CONTAINER (frame), gg->tform_ui.stage2_opt);

    /*
     * A button or two
    */
    hb = gtk_hbox_new (false, 5);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 0);

    btn = gtk_button_new_with_label ("Reset all");
    gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Set all transformation stages to 'no transformation' for the selected variables",
      NULL);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (tform_reset_cb), gg);

    gtk_widget_show_all (gg->tform_ui.window);
  }

  gdk_window_raise (gg->tform_ui.window->window);
}

/*
 * These routines are used to set the values of the option menus.
 * They're used when the transformations are set from somewhere
 * other than those option menus, such as the reset button.
*/
void
transform0_opt_menu_set_value (gint j, datad *d, ggobid *gg)
{
  gtk_option_menu_set_history (GTK_OPTION_MENU (gg->tform_ui.stage0_opt),
    d->vartable[j].tform0);
}
void
transform1_opt_menu_set_value (gint j, datad *d, ggobid *gg)
{
  gtk_option_menu_set_history (GTK_OPTION_MENU (gg->tform_ui.stage1_opt),
    d->vartable[j].tform1);
}
void
transform2_opt_menu_set_value (gint j, datad *d, ggobid *gg)
{
  gtk_option_menu_set_history (GTK_OPTION_MENU (gg->tform_ui.stage2_opt),
    d->vartable[j].tform2);
}
