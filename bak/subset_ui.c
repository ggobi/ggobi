/* subset_ui.c */

#include <gtk/gtk.h>
#include "vars.h"

static gboolean rescale_p = false;
static void rescalep_cb (GtkToggleButton *button)
{
  rescale_p = button->active;
}

static void
hide_cb (GtkWidget *w ) {
  gtk_widget_hide (w);
}
static void
subset_cb (GtkWidget *w ) {
  g_printerr ("perform the subset\n");
}
static void
include_all_cb (GtkWidget *w ) {
  g_printerr ("stop subsetting\n");
}

void set_start_incr_cb (GtkAdjustment *adj, GtkSpinButton *spin )
{
/*
 * Set the spin widget's adjustment->step_increment to adj->value
*/
  GtkAdjustment *adj_spin = gtk_spin_button_get_adjustment (spin);

  adj_spin->step_increment = adj->value;
  gtk_spin_button_set_adjustment (spin, adj_spin);
}

void set_block_incr_cb (GtkAdjustment *adj, GtkSpinButton *spin )
{
/*
 * Set the spin widget's adjustment->step_increment to adj->value
*/

  GtkAdjustment *adj_spin = gtk_spin_button_get_adjustment (spin);

  adj_spin->step_increment = adj->value;
  gtk_spin_button_set_adjustment (spin, adj_spin);
}

static GtkWidget *window = NULL;
void
open_subset_popup () {

  GtkWidget *notebook, *button, *t;
  GtkWidget *vbox, *frame, *hb, *vb;
  GtkWidget *label, *entry;
  GtkWidget *spinner, *start_spinner, *block_spinner;
  GtkAdjustment *adj;

  if (window == NULL) {
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "subset data");
    
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    
    /* Create a new notebook, place the position of the tabs */
    notebook = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
    gtk_box_pack_start (GTK_BOX (vbox), notebook, false, false, 2);
    
/*
 * Random sample without replacement
*/
    frame = gtk_frame_new ("Random sample without replacement");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

    hb = gtk_hbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (frame), hb);

    label = gtk_label_new ("Sample size");
    gtk_box_pack_start (GTK_BOX (hb), label, false, false, 2);
  
    entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (entry), 8);
    gtk_entry_set_text (GTK_ENTRY (entry), g_strdup_printf ("%d\n", xg.nrows));
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), entry,
      "Type in the desired sample size", NULL);
    gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

    label = gtk_label_new ("Random");
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
      
/*
 * Consecutive block
*/
    frame = gtk_frame_new ("Consecutive block");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

    t = gtk_table_new (2, 2, true);
    gtk_table_set_col_spacing (GTK_TABLE (t), 0, 20);
    gtk_container_set_border_width (GTK_CONTAINER (t), 10);
    gtk_container_add (GTK_CONTAINER (frame), t);

    /* First case */
    vb = gtk_vbox_new (false, 3);
    label = gtk_label_new ("First case:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

    adj = (GtkAdjustment *) gtk_adjustment_new (1.0,
                          1.0, (gfloat) xg.nrows, 1.0, 5.0, 0.0);
    start_spinner = gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (start_spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (start_spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), start_spinner,
      "Specify the first row of the block", NULL);
    gtk_box_pack_start (GTK_BOX (vb), start_spinner, false, false, 0);
    gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,0,1);

    /* blocksize */
    vb = gtk_vbox_new (false, 2);
    label = gtk_label_new ("Blocksize:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

    adj = (GtkAdjustment *) gtk_adjustment_new ( (gfloat) xg.nrows/10.0,
                          1.0, (gfloat) xg.nrows, 1.0, 5.0, 0.0);
    block_spinner = gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (block_spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (block_spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), block_spinner,
      "Specify the size of the block", NULL);
    gtk_box_pack_start (GTK_BOX (vb), block_spinner, false, false, 0);
    gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,0,1);

    /* First case increment */
    vb = gtk_vbox_new (false, 2);
    label = gtk_label_new ("Increment:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

    adj = (GtkAdjustment *) gtk_adjustment_new ( 1.0,
                          1.0, (gfloat) xg.nrows, 1.0, 5.0, 0.0);
    spinner = gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), spinner,
      "Specify the size of the increment for the arrows used to increment and decrement the first case, just above", NULL);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (set_start_incr_cb),
		      (gpointer) start_spinner);
    gtk_box_pack_start (GTK_BOX (vb), spinner, false, false, 0);
    gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,1,2);

    /* Blocksize increment */
    vb = gtk_vbox_new (false, 2);
    label = gtk_label_new ("Increment:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

    adj = (GtkAdjustment *) gtk_adjustment_new ( 1.0,
                          1.0, (gfloat) xg.nrows, 1.0, 5.0, 0.0);
    spinner = gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), spinner,
      "Specify the size of the increment for the arrows used to increment and decrement the blocksize, just above", NULL);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		                GTK_SIGNAL_FUNC (set_block_incr_cb),
		                (gpointer) block_spinner);
    gtk_box_pack_start (GTK_BOX (vb), spinner, false, false, 0);
    gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,1,2);

    label = gtk_label_new ("Block");
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

/*
 * Every nth case
*/
    frame = gtk_frame_new ("Every nth case");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

    t = gtk_table_new (1, 2, true);
    gtk_table_set_col_spacing (GTK_TABLE (t), 0, 20);
    gtk_container_set_border_width (GTK_CONTAINER (t), 10);
    gtk_container_add (GTK_CONTAINER (frame), t);

    /* First case */
    vb = gtk_vbox_new (false, 3);
    label = gtk_label_new ("First case:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

    adj = (GtkAdjustment *) gtk_adjustment_new (1.0,
                          1.0, (gfloat) xg.nrows-1, 1.0, 5.0, 0.0);
    start_spinner = gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (start_spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (start_spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), start_spinner,
      "Specify the first row of the block", NULL);
    gtk_box_pack_start (GTK_BOX (vb), start_spinner, false, false, 0);
    gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,0,1);

    /* blocksize */
    vb = gtk_vbox_new (false, 2);
    label = gtk_label_new ("N:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

    adj = (GtkAdjustment *) gtk_adjustment_new ( (gfloat) xg.nrows/10.0,
                          1.0, (gfloat) xg.nrows-1, 1.0, 5.0, 0.0);
    block_spinner = gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (block_spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (block_spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), block_spinner,
      "Specify the size of the block", NULL);
    gtk_box_pack_start (GTK_BOX (vb), block_spinner, false, false, 0);
    gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,0,1);

    label = gtk_label_new ("Every n");
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

/*
 * Cases whose row label is one of the sticky labels
*/
    frame = gtk_frame_new ("Cases whose row label is sticky");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
    gtk_widget_set_usize (frame, 100, 75);
    label = gtk_label_new ("Sticky");
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

/*
 * Cases whose row label the specified row label
*/
    frame = gtk_frame_new ("Cases with specified row label");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

    hb = gtk_hbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (frame), hb);

    label = gtk_label_new ("Row label:");
    gtk_box_pack_start (GTK_BOX (hb), label, false, false, 2);
  
    entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (entry), 32);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), entry,
      "Type in the label shared by the cases you want in the subset",
      NULL);
    gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

    label = gtk_label_new ("Row label");
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

/* 
 * hbox to hold a few buttons
*/
    
    hb = gtk_hbox_new (true, 2);

    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 2);

    button = gtk_button_new_with_label ("Subset");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), spinner,
      "Draw a new subset and update all plots", NULL);
    gtk_signal_connect (GTK_OBJECT (button),
                        "clicked",
                        GTK_SIGNAL_FUNC (subset_cb),
                        (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (hb), button, true, true, 2);

    button = gtk_check_button_new_with_label ("Rescale");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), button,
                          "Rescale the data when drawing a new subset", NULL);
    gtk_signal_connect (GTK_OBJECT (button), "toggled",
                        GTK_SIGNAL_FUNC (rescalep_cb), (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (hb), button, true, true, 2);
    
    button = gtk_button_new_with_label ("Include all");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), spinner,
      "Stop subsetting: include all cases and update all plots", NULL);
    gtk_signal_connect (GTK_OBJECT (button),
                        "clicked",
                        GTK_SIGNAL_FUNC (include_all_cb),
                        (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (hb), button, true, true, 2);

/*
 * Close button
*/
    button = gtk_button_new_with_label ("Close");
    gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                   GTK_SIGNAL_FUNC (hide_cb), (gpointer) window);
    gtk_box_pack_start (GTK_BOX (vbox), button, false, true, 2);
  }

    gtk_widget_show_all (window);
}
