/* transform_ui.c */

#include <gtk/gtk.h>
#include "vars.h"

/* external functions */
extern void populate_option_menu (GtkWidget *, gchar **, gint,
                                  GtkSignalFunc);
/*                    */

static void
hide_cb (GtkWidget *w ) {
  gtk_widget_hide (w);
}

static gchar *stage1_lbl[] = {"No transformation",
                              "Raise minimum to 0",
                              "Raise minimum to 1",
                              "Negative"};
static void stage1_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", stage1_lbl[indx]);
}

static gchar *stage2_lbl[] = {"No transformation",
                              "Box-Cox",
                              "Absolute value",
                              "Inverse",
                              "Log base 10",
                              "Scale to [0,1]",
                              "Discretize: 2 levels",
                              "Discretize: Normal score"};
static void stage2_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", stage2_lbl[indx]);
}
void boxcox_cb (GtkAdjustment *adj, GtkWidget *spin )
{
/*
 * Set the spin widget's adjustment->step_increment to adj->value
*/
  g_printerr ("%f\n", adj->value);
}

static gchar *stage3_lbl[] = {"No transformation",
                              "Standardize",
                              "Permute",
                              "Sort",
                              "Principal components"};
static void stage3_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", stage3_lbl[indx]);
}

static void reset_tform_cb (GtkButton *button)
{
  g_printerr ("Remove all transformations\n");
}

static GtkWidget *window = NULL;
void
open_transform_popup () {

  GtkWidget *lbl, *hbox;
  GtkWidget *vbox, *frame, *hb, *vb, *btn;
  GtkWidget *opt, *spinner;
  GtkAdjustment *adj;

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
     * Stage 1: Domain adjustment
    */
    frame = gtk_frame_new ("Stage 1");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_box_pack_start (GTK_BOX (vbox), frame, true, false, 1);

    opt = gtk_option_menu_new ();
    gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
      "Stage 1: Adjust the domain of the variables", NULL);
    populate_option_menu (opt, stage1_lbl,
                          sizeof (stage1_lbl) / sizeof (gchar *),
                          stage1_cb);
    gtk_container_add (GTK_CONTAINER (frame), opt);

    /*
     * Stage 2: Power transformations et al
    */
    frame = gtk_frame_new ("Stage 2");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_box_pack_start (GTK_BOX (vbox), frame, true, false, 1);

    vb = gtk_vbox_new (false, 5);
    gtk_container_add (GTK_CONTAINER (frame), vb);

    opt = gtk_option_menu_new ();
    gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
      "Stage 2: Power transformations et al", NULL);
    populate_option_menu (opt, stage2_lbl,
                          sizeof (stage2_lbl) / sizeof (gchar *),
                          stage2_cb);
    gtk_box_pack_start (GTK_BOX (vb), opt, true, false, 1);

    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vb), hb, true, false, 2);
    
    lbl = gtk_label_new ("Box-Cox parameter:");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);
    adj = (GtkAdjustment *) gtk_adjustment_new (1.0,
                          -4, 5, 0.05, .5, 0.0);
    spinner = gtk_spin_button_new (adj, 0, 3);

    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), spinner,
      "Set the Box-Cox power function parameter", NULL);
    gtk_box_pack_end (GTK_BOX (hb), spinner, true, true, 0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		                GTK_SIGNAL_FUNC (boxcox_cb),
		                NULL);

    /*
     * Stage 3: Sorting and permutation
    */
    frame = gtk_frame_new ("Stage 3");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_box_pack_start (GTK_BOX (vbox), frame, true, false, 1);

    opt = gtk_option_menu_new ();
    gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
      "Stage 3: Permutation, sorting, and sphering", NULL);
    populate_option_menu (opt, stage3_lbl,
                          sizeof (stage3_lbl) / sizeof (gchar *),
                          stage3_cb);
    gtk_container_add (GTK_CONTAINER (frame), opt);

    /*
     * A button or two
    */
    hb = gtk_hbox_new (false, 5);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 0);

    btn = gtk_button_new_with_label ("Reset all");
    gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
      "Set all transformation stages to 'no transformation' for the selected variables", NULL);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (reset_tform_cb), NULL);

  }

  gtk_widget_show_all (window);
}
