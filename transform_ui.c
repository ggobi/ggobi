/* transform_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static void
hide_cb (GtkWidget *w ) {
  gtk_widget_hide (w);
}

static gchar *stage0_lbl[] = {"No transformation",
                              "Raise minimum to 0",
                              "Raise minimum to 1",
                              "Negative"};
static void stage0_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  ggobid *gg = GGobiFromWidget(w, true);
  transform (0, indx, -99., gg);
}

static gchar *stage1_lbl[] = {"No transformation",
                              "Standardize",
                              "Box-Cox",
                              "Absolute value",
                              "Inverse",
                              "Log base 10",
                              "Scale to [0,1]",
                              "Discretize: 2 levels",
                              "Rank",
                              "Normal score",
                              "Z-score"
                              };
static void
stage1_cb (GtkWidget *w, gpointer cbd)
{
 ggobid *gg;
  gint indx = GPOINTER_TO_INT (cbd);
  gg = GGobiFromWidget(w, true);
  transform (1, indx, gg->app.boxcox_adj->value, gg);
}

/*
 * Set the spin widget's adjustment->step_increment to adj->value
*/
void boxcox_cb (GtkAdjustment *adj, ggobid *gg)
{
  transform (1, BOXCOX, adj->value, gg);
}

static gchar *stage2_lbl[] = {"No transformation",
                              "Standardize",
                              "Permute",
                              "Sort",
                              "Sphere ..."};
static void stage2_cb (GtkWidget *w, gpointer cbd)
{
 ggobid *gg;
  gint indx = GPOINTER_TO_INT (cbd);
  gg = GGobiFromWidget(w, true);

  if (indx == SPHERE) {
    sphere_panel_open (gg);
  } else {
    transform (2, indx, -99, gg);
  }
}


static void tform_reset_cb (GtkButton *button)
{
  g_printerr ("Remove all transformations\n");
}

static GtkWidget *window = NULL;
void
transform_window_open (ggobid *gg) 
{

  GtkWidget *lbl, *hbox;
  GtkWidget *vbox, *frame, *hb, *vb, *btn;
  GtkWidget *spinner;

  if (gg->nrows == 0)  /*-- if used before we have data --*/
    return;

  if (window == NULL) {
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "transform variables");
    
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

/*
 * Divide the window:  functions on the left, labels on the right
*/
    hbox = gtk_hbox_new (false, 1);
    gtk_container_border_width (GTK_CONTAINER (hbox), 1);
    gtk_container_add (GTK_CONTAINER (window), hbox);

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

    gg->app.stage0_opt = gtk_option_menu_new ();
    gtk_container_set_border_width (GTK_CONTAINER (gg->app.stage0_opt), 4);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->app.stage0_opt,
      "Stage 1: Adjust the domain of the variables",
      NULL);
    populate_option_menu (gg->app.stage0_opt, stage0_lbl,
                          sizeof (stage0_lbl) / sizeof (gchar *),
                          stage0_cb, gg);
    gtk_container_add (GTK_CONTAINER (frame), gg->app.stage0_opt);

    /*
     * Stage 1: Power transformations et al
    */
    frame = gtk_frame_new ("Stage 1");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame, true, false, 1);

    vb = gtk_vbox_new (false, 5);
    gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
    gtk_container_add (GTK_CONTAINER (frame), vb);

    gg->app.stage1_opt = gtk_option_menu_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->app.stage1_opt,
      "Stage 2: Power transformations et al",
      NULL);
    populate_option_menu (gg->app.stage1_opt, stage1_lbl,
                          sizeof (stage1_lbl) / sizeof (gchar *),
                          stage1_cb, gg);
    gtk_box_pack_start (GTK_BOX (vb), gg->app.stage1_opt, true, false, 1);

    hb = gtk_vbox_new (false, 2);  /*-- changed from hbox to vbox --*/
    gtk_box_pack_start (GTK_BOX (vb), hb, true, false, 2);
    
    lbl = gtk_label_new ("Box-Cox parameter:");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);
    gg->app.boxcox_adj = (GtkAdjustment *) gtk_adjustment_new (1.0,
                          -4, 5, 0.05, .5, 0.0);
    spinner = gtk_spin_button_new (gg->app.boxcox_adj, 0, 3);

    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinner,
      "Set the Box-Cox power function parameter", NULL);
    gtk_box_pack_end (GTK_BOX (hb), spinner, true, true, 0);
    gtk_signal_connect (GTK_OBJECT (gg->app.boxcox_adj), "value_changed",
		                GTK_SIGNAL_FUNC (boxcox_cb),
		                (gpointer) gg);

    /*
     * Stage 2: Sorting and permutation
    */
    frame = gtk_frame_new ("Stage 2");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame, true, false, 1);

    gg->app.stage2_opt = gtk_option_menu_new ();
    gtk_container_set_border_width (GTK_CONTAINER (gg->app.stage2_opt), 4);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->app.stage2_opt,
      "Stage 3: Permutation, sorting, and sphering", NULL);
    populate_option_menu (gg->app.stage2_opt, stage2_lbl,
                          sizeof (stage2_lbl) / sizeof (gchar *),
                          stage2_cb, gg);
    gtk_container_add (GTK_CONTAINER (frame), gg->app.stage2_opt);

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
                      GTK_SIGNAL_FUNC (tform_reset_cb), NULL);

    gtk_widget_show_all (window);
  }

  gdk_window_raise (window->window);
}

/*
 * I'm a bit bewildered about this at the moment, but I guess its
 * purpose might be to handle the case where the transformation is
 * executed through the api
*/
void
transform_opt_menus_set_history (gint j, ggobid *gg)
{
  gtk_option_menu_set_history (GTK_OPTION_MENU (gg->app.stage0_opt),
    gg->vardata[j].tform0);
  gtk_option_menu_set_history (GTK_OPTION_MENU (gg->app.stage1_opt),
    gg->vardata[j].tform1);
  gtk_option_menu_set_history (GTK_OPTION_MENU (gg->app.stage2_opt),
    gg->vardata[j].tform2);
}
