/* subset_ui.c */

#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define SS_RANDOM 0
#define SS_BLOCK  1
#define SS_EVERYN 2
#define SS_STICKY 3
#define SS_ROWLAB 4

extern void subset_include_all (ggobid *);
extern void subset_apply (gboolean, ggobid *);
extern gboolean subset_random (gint, ggobid *);
extern gboolean subset_block (gint, gint, ggobid *);
extern gboolean subset_everyn (gint, gint, ggobid *);
extern gboolean subset_sticky (ggobid *gg);
extern gboolean subset_rowlab (gchar *, ggobid *gg);


static void
delete_cb (GtkWidget *w, GdkEventButton *event, gpointer data) {
  gtk_widget_hide (w);
}

static void rescalep_cb (GtkToggleButton *button, ggobid *gg)
{
  gg->subset.rescale_p = button->active;
}

static void
subset_cb (GtkWidget *w, ggobid *gg) {
  gint subset_type;
  gint sample_size;
  gchar *sample_str, *rowlab;
  gint bstart, bsize;
  gint estart, estep;
  gboolean redraw;

  subset_type = 
    gtk_notebook_get_current_page (GTK_NOTEBOOK (gg->subset.notebook));

  switch (subset_type) {
    case SS_RANDOM:
      sample_str = 
        gtk_editable_get_chars (GTK_EDITABLE (gg->subset.random_entry),
                                           0, -1);
      sample_size = atoi (sample_str);
      redraw = subset_random (sample_size, gg);
      break;
    case SS_BLOCK:
      bstart = (gint) gg->subset.bstart_adj->value;
      bsize = (gint) gg->subset.bsize_adj->value;
      redraw = subset_block (bstart-1, bsize, gg);
      break;
    case SS_EVERYN:
      estart = (gint) gg->subset.estart_adj->value;
      estep = (gint) gg->subset.estep_adj->value;
      redraw = subset_everyn (estart-1, estep, gg);
      break;
    case SS_STICKY:
      redraw = subset_sticky (gg);
      break;
    case SS_ROWLAB:
      rowlab =
        gtk_editable_get_chars (GTK_EDITABLE (gg->subset.rowlab_entry),
        0, -1);
      redraw = subset_rowlab (rowlab, gg);
      break;
  }

  if (redraw)
    subset_apply (gg->subset.rescale_p, gg);
}

static void
include_all_cb (GtkWidget *w, ggobid *gg) {
  subset_include_all (gg);

  subset_apply (gg->subset.rescale_p, gg);
}

/*------------------------------------------------------------------*/
/*     adjusting the increments for block and everyn subsetting     */
/*------------------------------------------------------------------*/

/*-- Set the increment for block_start --*/
void set_start_incr_cb (GtkAdjustment *adj, GtkSpinButton *spin )
{
/*-- Set the spin widget's adjustment->step_increment to adj->value --*/
  GtkAdjustment *adj_spin = gtk_spin_button_get_adjustment (spin);

  adj_spin->step_increment = adj->value;
  gtk_spin_button_set_adjustment (spin, adj_spin);
}

/*-- Set the increment for block_size --*/
void set_block_incr_cb (GtkAdjustment *adj, GtkSpinButton *spin )
{
/*-- Set the spin widget's adjustment->step_increment to adj->value --*/
  GtkAdjustment *adj_spin = gtk_spin_button_get_adjustment (spin);

  adj_spin->step_increment = adj->value;
  gtk_spin_button_set_adjustment (spin, adj_spin);
}

/*------------------------------------------------------------------*/

/*
 * note that this uses d->nrows.  What happens if it remains open
 * and the value of d is reset?
*/
void
subset_window_open (ggobid *gg) {

  GtkWidget *button, *t;
  GtkWidget *vbox, *frame, *hb, *vb;
  GtkWidget *label;
  GtkWidget *spinner, *start_spinner, *block_spinner;
  GtkAdjustment *adj;
  datad *d = gg->current_display->d;
  gfloat fnr = (gfloat) d->nrows;

  if (d->nrows == 0)  /*-- if used before we have data --*/
    return;

  if (gg->subset.window == NULL) {
    
    gg->subset.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->subset.window),
      "subset data");
    gtk_signal_connect (GTK_OBJECT (gg->subset.window),
      "delete_event", GTK_SIGNAL_FUNC (delete_cb), NULL);
  
    gtk_container_set_border_width (GTK_CONTAINER (gg->subset.window),
      5);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (gg->subset.window), vbox);
    
    /* Create a new notebook, place the position of the tabs */
    gg->subset.notebook = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gg->subset.notebook),
      GTK_POS_TOP);
    gtk_box_pack_start (GTK_BOX (vbox), gg->subset.notebook,
      false, false, 2);
    
/*
 * Random sample without replacement
*/
    frame = gtk_frame_new ("Random sample without replacement");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

    hb = gtk_hbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (frame), hb);

    label = gtk_label_new ("Sample size");
    gtk_box_pack_start (GTK_BOX (hb), label, false, false, 2);
  
    gg->subset.random_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (gg->subset.random_entry), 8);
    gtk_entry_set_text (GTK_ENTRY (gg->subset.random_entry),
      g_strdup_printf ("%d", d->nrows));
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->subset.random_entry,
      "Type in the desired sample size", NULL);
    gtk_box_pack_start (GTK_BOX (hb), gg->subset.random_entry,
      true, true, 2);

    label = gtk_label_new ("Random");
    gtk_notebook_append_page (GTK_NOTEBOOK
      (gg->subset.notebook), frame, label);
      
/*
 * Consecutive block
*/
    frame = gtk_frame_new ("Consecutive block");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

    t = gtk_table_new (2, 2, true);
    gtk_table_set_col_spacing (GTK_TABLE (t), 0, 20);
    gtk_container_set_border_width (GTK_CONTAINER (t), 5);
    gtk_container_add (GTK_CONTAINER (frame), t);

    /* Block subsetting: First case */
    vb = gtk_vbox_new (false, 3);
    label = gtk_label_new ("First case:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

    gg->subset.bstart_adj = (GtkAdjustment *)
      gtk_adjustment_new (1.0, 1.0, (fnr-2.0), 1.0, 5.0, 0.0);
    start_spinner = gtk_spin_button_new (gg->subset.bstart_adj, 0, 0);

    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (start_spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (start_spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), start_spinner,
      "Specify the first row of the block", NULL);
    gtk_box_pack_start (GTK_BOX (vb), start_spinner, false, false, 0);
    gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,0,1);

    /* Block subsetting: blocksize */
    vb = gtk_vbox_new (false, 2);
    label = gtk_label_new ("Blocksize:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

    gg->subset.bsize_adj = (GtkAdjustment *)
      gtk_adjustment_new (fnr/10.0, 1.0, fnr, 1.0, 5.0, 0.0);
    block_spinner = gtk_spin_button_new (gg->subset.bsize_adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (block_spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (block_spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), block_spinner,
      "Specify the size of the block", NULL);
    gtk_box_pack_start (GTK_BOX (vb), block_spinner, false, false, 0);
    gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,0,1);

    /* First case increment */
    vb = gtk_vbox_new (false, 2);
    label = gtk_label_new ("Increment:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

    adj = (GtkAdjustment *) gtk_adjustment_new ( 1.0,
                          1.0, fnr, 1.0, 5.0, 0.0);
    spinner = gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinner,
      "Specify the size of the increment for the arrows used to increment and decrement the first case, just above",
      NULL);
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
                          1.0, fnr, 1.0, 5.0, 0.0);
    spinner = gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinner,
      "Specify the size of the increment for the arrows used to increment and decrement the blocksize, just above", NULL);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		                GTK_SIGNAL_FUNC (set_block_incr_cb),
		                (gpointer) block_spinner);
    gtk_box_pack_start (GTK_BOX (vb), spinner, false, false, 0);
    gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,1,2);

    label = gtk_label_new ("Block");
    gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset.notebook), frame, label);

/*
 * Every nth case
*/
    frame = gtk_frame_new ("Every nth case");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

    t = gtk_table_new (1, 2, true);
    gtk_table_set_col_spacing (GTK_TABLE (t), 0, 20);
    gtk_container_set_border_width (GTK_CONTAINER (t), 5);
    gtk_container_add (GTK_CONTAINER (frame), t);

    /* everyn subsetting: start */
    vb = gtk_vbox_new (false, 3);
    label = gtk_label_new ("First case:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

    gg->subset.estart_adj = (GtkAdjustment *) gtk_adjustment_new (1.0,
                          1.0, fnr-2.0, 1.0, 5.0, 0.0);
    start_spinner = gtk_spin_button_new (gg->subset.estart_adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (start_spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (start_spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), start_spinner,
      "Specify the first row of the block", NULL);
    gtk_box_pack_start (GTK_BOX (vb), start_spinner, false, false, 0);
    gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,0,1);

    /* everyn subsetting: stepsize */
    vb = gtk_vbox_new (false, 2);
    label = gtk_label_new ("N:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

    gg->subset.estep_adj = (GtkAdjustment *)
      gtk_adjustment_new (fnr/10.0, 1.0, fnr-1, 1.0, 5.0, 0.0);
    block_spinner = gtk_spin_button_new (gg->subset.estep_adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (block_spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (block_spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), block_spinner,
      "Specify the size of the block", NULL);
    gtk_box_pack_start (GTK_BOX (vb), block_spinner, false, false, 0);
    gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,0,1);

    label = gtk_label_new ("Every n");
    gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset.notebook),
      frame, label);

/*
 * Cases whose row label is one of the sticky labels
*/
    frame = gtk_frame_new ("Cases whose row label is sticky");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_widget_set_usize (frame, 100, 75);
    label = gtk_label_new ("Sticky");
    gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset.notebook),
      frame, label);

/*
 * Cases whose row label the specified row label
*/
    frame = gtk_frame_new ("Cases with specified row label");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

    hb = gtk_hbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (frame), hb);

    label = gtk_label_new ("Row label:");
    gtk_box_pack_start (GTK_BOX (hb), label, false, false, 2);
  
    gg->subset.rowlab_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (gg->subset.rowlab_entry), 32);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->subset.rowlab_entry,
      "Type in the label shared by the cases you want in the subset",
      NULL);
    gtk_box_pack_start (GTK_BOX (hb),
      gg->subset.rowlab_entry, true, true, 2);

    label = gtk_label_new ("Row label");
    gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset.notebook),
      frame, label);

/* 
 * hbox to hold a few buttons
*/
    
    hb = gtk_hbox_new (true, 2);

    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 2);

    button = gtk_button_new_with_label ("Subset");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinner,
      "Draw a new subset and update all plots", NULL);
    gtk_signal_connect (GTK_OBJECT (button),
                        "clicked",
                        GTK_SIGNAL_FUNC (subset_cb),
                        (gpointer) gg);
    gtk_box_pack_start (GTK_BOX (hb), button, true, true, 2);

    button = gtk_check_button_new_with_label ("Rescale");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
                          "Rescale the data when drawing a new subset", NULL);
    gtk_signal_connect (GTK_OBJECT (button), "toggled",
                        GTK_SIGNAL_FUNC (rescalep_cb), (gpointer) gg);
    gtk_box_pack_start (GTK_BOX (hb), button, true, true, 2);
    
    button = gtk_button_new_with_label ("Include all");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinner,
      "Stop subsetting: include all cases and update all plots", NULL);
    gtk_signal_connect (GTK_OBJECT (button),
                        "clicked",
                        GTK_SIGNAL_FUNC (include_all_cb),
                        (gpointer) &gg);
    gtk_box_pack_start (GTK_BOX (hb), button, true, true, 2);
  }

    gtk_widget_show_all (gg->subset.window);
}
